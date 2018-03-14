/** \file asciiProgram.cpp
*	Author: Trenton Wells
*
*	Created: July 29, 2016, 3:29 PM
*/

#include "asciiProgram.h"
#include <functional>

static int gateInterupt = 0;
static bool graphing = false;
static bool stp_move_done = false;
QElapsedTimer asciiProgram::cslul;
int asciiProgram::cycVal;

asciiProgram::asciiProgram(QObject *parent) : QThread(parent)
{
    parentProgram = parent;
}

/** asciiProgram destructor. */
asciiProgram::~asciiProgram()
{
	delete COM;
    delete IAI;
    delete IIC;
    delete STP;
    delete timerAscii;
}

/**
 * @brief asciiProgram::run Overload thread function.
 * This function is an infinate loop that contains the functions needed for the program thread.
 * Function calls are made through the signals and slots method.
 */
void asciiProgram::run()
{
    COM = new serialPortManager();
    IAI = new actuator();
    IIC = new i2c();
    STP = new stepper();
    timerAscii = new timeManager();
    COM->start();
    //DTX->start();

    //----------------------- Qt signials and slots  connections ----------------------//
    QObject::connect(IAI, SIGNAL(sigMsgOut(QByteArray)), COM, SLOT(sltSerialMsgIn(QByteArray)));
    QObject::connect(COM, SIGNAL(sigSerialMsgOut(QByteArray)), IAI, SLOT(sltMsgIn(QByteArray)));
    QObject::connect(this, SIGNAL(sigWriteXml3D(QVector<float>, QVector<float>, QVector<uint>, QVector<uint>, uint, uint)), DTX, SLOT(sltWriteXml3D(QVector<float>, QVector<float>, QVector<uint>, QVector<uint>, uint, uint)));
    QObject::connect(this, SIGNAL(sigDataOut(QStringList, QStringList, QStringList, QStringList)), DTX, SLOT(sltDataIn(QStringList, QStringList, QStringList, QStringList)));
    QObject::connect(this, SIGNAL(sigWarningMsg(QString)), parentProgram, SLOT(sltWarningMsg(QString)));
    QObject::connect(this, SIGNAL(sigStatusMsg(QString,int)), parentProgram, SLOT(sltStatusMsg(QString,int)));
    QObject::connect(parentProgram, SIGNAL(sigIAIServoOnOff(int)), IAI, SLOT(sltServoOnOff(int)));
    QObject::connect(parentProgram, SIGNAL(sigIAIHome()), IAI, SLOT(sltHome()));
    QObject::connect(parentProgram, SIGNAL(sigIAIJogPos()), IAI, SLOT(sltJogPos()));
    QObject::connect(parentProgram, SIGNAL(sigIAIJogPosStop()), IAI, SLOT(sltJogPosStop()));
    QObject::connect(parentProgram, SIGNAL(sigIAIJogNeg()), IAI, SLOT(sltJogNeg()));
    QObject::connect(parentProgram, SIGNAL(sigIAIJogNegStop()), IAI, SLOT(sltJogNegStop()));
    QObject::connect(parentProgram, SIGNAL(sigStepperHome()), STP, SLOT(sltStepperHome()));
    QObject::connect(parentProgram, SIGNAL(sigStepperJogPos(bool, bool)), STP, SLOT(sltStepperJogPos(bool, bool)));
    QObject::connect(parentProgram, SIGNAL(sigStepperJogNeg(bool, bool)), STP, SLOT(sltStepperJogPos(bool, bool)));
    QObject::connect(parentProgram, SIGNAL(sigResponseTimeProg()), this, SLOT(sltRunProg(controlSettings)));
    QObject::connect(parentProgram, SIGNAL(sigBoundryProg()), this, SLOT(sltRunProg(controlSettings)));
    QObject::connect(parentProgram, SIGNAL(sigOffSurfaceProg()), this, SLOT(sltRunProg(controlSettings)));
    QObject::connect(parentProgram, SIGNAL(sigFullTestProg()), this, SLOT(sltRunProg(controlSettings)));
    QObject::connect(parentProgram, SIGNAL(sigStepperJogPos(bool, bool)), this, SLOT(sltStepperJogPos(bool, bool)));
    QObject::connect(parentProgram, SIGNAL(sigStepperJogNeg(bool, bool)), this, SLOT(sltStepperJogNeg(bool, bool)));
    QObject::connect(parentProgram, SIGNAL(sigManuVals(float, float, float, float, float, float, float, float, QString, float, int)), this, SLOT(sltManuVals(float, float, float, float, float, float, float, float, QString, float, int)));
    QObject::connect(parentProgram, SIGNAL(sigManuValsControl(bool)), this, SLOT(sltManuValsControl(bool)));
    QObject::connect(parentProgram, SIGNAL(sigCBRModeChange(uint)), this, SLOT(sltCBRModeChange(uint)));
    QObject::connect(parentProgram, SIGNAL(sigCBRManualChange(uint)), this, SLOT(sltCBRManualChange(uint)));
    QObject::connect(parentProgram, SIGNAL(sigWindowValRunning(bool)), this, SLOT(sltValRunning(bool)));
    QObject::connect(parentProgram, SIGNAL(sigHandleMODE(QString)), this, SLOT(sltHandleMODE(QString)));

    stepTimer.setInterval(1);
    /** WiringPi code. This code sets up the program to access the GPIO. Need to use root! */
    /**  Relevant files can be found in /home/pi/gpio_init.sh, remember to call
        sudo update-rc.d gpio_init.sh defaults											   */
    wiringPiSetupSys();

    //set up the interrupt isr for wiringpi
    wiringPiISR(GATE_PIN, INT_EDGE_BOTH, &myInterrupt);
    wiringPiISR(MOTION_DONE, INT_EDGE_BOTH, &myInterrupt2);
    stp_move_done = false;
    HandleMODE = LOCK;

    this->exec();
}

void asciiProgram::sltRunProg(controlSettings &progSettings)
{
    float handleSurface = 0;

    if (CBRModeSelected == 0)
    {
        emit sigWarningMsg("Select Handle Mode Before Starting.");
    }
    else
    {
        emit sigStatusMsg(progSettings.programName, 3000);
        IAI->servoON();  // Turn the IAI motor controller to on.
        timerAscii->waitMs(400); // Wait for it to settle
        IAI->HOME(); // Home the IAI actuator.

        handleSurface = findHandleLocation();
        //CBRModeSelected = progSettings.CBRModeVal;
        CBRMode(false);
        timerAscii->waitMs(200);

        if ("offSurfaceProg" == progSettings.programName)
        {
            offSurfaceProg(handleSurface, progSettings);
        }
        else if ("threeDBoundaryTestProg" == progSettings.programName)
        {
            threeDBoundaryProg(handleSurface, progSettings);
        }
        else if ("fullTestProg" == progSettings.programName)
        {
            fullTestProg(handleSurface, progSettings);
        }
        else
        {
            emit sigWarningMsg("ERROR: No function name found in sltRunProg(controlSettings &progSettings)... progSettings.programName == " + progSettings.programName);
        }
    }

    IAI->servoOFF();
    CBRMode(true);

    emit sigStatusMsg("Run complete.", 10000);
}

/** \brief Sets the handle activation mode.
*	\param mode the mode that is desired.
*	Recieves signial with param.
*/
void asciiProgram::sltHandleMODE(QString mode)
{
	if (mode == "LOCK")
	{
		HandleMODE = LOCK;
	}
	else
	{
		HandleMODE = UNLOCK;
	}
}

/**
 * @brief asciiProgram::sltManuVals
 * @param currentTargetIn
 * @param currentPositioningIn
 * @param currentSpeedIn
 * @param currentZonePosIn
 * @param currentZoneNegIn
 * @param currentAccelerationIn
 * @param currentDeaccelerationIn
 * @param currentPushIn
 * @param currentCtlrFlagIn
 * @param currentOffsetIn
 * @param currentCycleIn
 */
void asciiProgram::sltManuVals(float currentTargetIn, float currentPositioningIn,
    float currentSpeedIn, float currentZonePosIn,
    float currentZoneNegIn, float currentAccelerationIn,
    float currentDeaccelerationIn, float currentPushIn,
    QString currentCtlrFlagIn, float currentOffsetIn,
                               int currentCycleIn)
{
    currentTarget = currentTargetIn;
    currentPositioning = currentPositioningIn;
    currentSpeed = currentSpeedIn;
	currentZonePos = currentZonePosIn;
	currentZoneNeg = currentZoneNegIn;
    currentAcceleration = currentAccelerationIn;
	currentDeacceleration = currentDeaccelerationIn;
    currentPush = currentPushIn;
	currentCtlrFlag = currentCtlrFlagIn;
    currentOffset = currentOffsetIn;
    currentCycle = currentCycleIn;
}

/**
 * @brief asciiProgram::sltManuValsControl
 * @param running
 */
void asciiProgram::sltManuValsControl(bool running)
{
    emit sigStatusMsg("manualModeControl()" + QString::number(running), 4000);
    manualRun = running;
    if (manualRun == true)
    {
        manualModeControl();
    }
}

/**
 * @brief asciiProgram::sltValRunning
 * @param value
 */
void asciiProgram::sltValRunning(bool value)
{
	running = value;
}

/**
 * @brief asciiProgram::findHandleLocation
 * Locates the handle by performing a push operation across the entire work area
 * This assumes that the IAI has run home and is currently on.
 * The reason for this is to limit sending the same commands more than once per test cycle.
 * The actuator must also have a very thin conductive foam tip covering the probe to not damage the handle.
 * @return float handle location in mm
 */
float asciiProgram::findHandleLocation()
{
    float handleLocation = 0;
    QByteArray request = "";

    IAI->messageFrame(IAI->pushToMsg());

    // PSFL: Address 9005 bit 11
    // Missed work part in push-motion operation.
    // aka: didn't hit an object.
    // Bit turns 1 when missed.

    // TRQS: Address 9006 bit 12
    // Torque level status
    // Bit turns 1 when current has reached a level corresponding to the push torque levels.

    request.append(IAI_SLAVE_ADDRESS);
    request.append(IAI_CODE_READ);
    request.append("9005");
    request.append("0002");

    timerAscii->waitMs(300);

    IAI->dataReturn.clear();

    timerAscii->startTimeWhile();

    while (timerAscii->endTimeWhile() < 8000)
    {
        IAI->messageFrame(request);
        timerAscii->waitMs(100);
        if (IAI->dataReturn.length() >= 8)
        {
            bool ok = true;
            uint datas = IAI->dataReturn.toInt(&ok, 16);
            datas = datas & 0x8001000;

            if (datas == 0x8000000)
            {
                break;
            }
            else if (datas == 0x1000)
            {
                handleLocation = IAI->posNOW();
            }
            else
            {
                // misra compliance.
            }
        }
        else
        {
            // misra compliance
        }
    }

    if (handleLocation >= 149.9)
    {
        handleLocation = 0;
    }

    emit sigStatusMsg(QString::number(handleLocation) + " handleLocation", 3000);

    IAI->moveTo(0,0.01);

    //qDebug() << "next";

    return handleLocation;
}

float asciiProgram::offSurfaceProg(float handleSurface, controlSettings &progSettings)
{
    float handleOffset = 0;
    QByteArray boundaryOffset;
    int loopActivations = 0;
    int handleMode = HandleMODE;
    bool boundaryLimit = false;

    boundaryOffset = IAI->moveToMsg(handleSurface, 0.01, progSettings.currentSpeed, progSettings.currentAcceleration);

    while(!boundaryLimit)
    {
        for (int i = 0; i < progSettings.offSurfaceProgSettings.loopCount; i++)
        {
            IAI->messageFrame(boundaryOffset);
            timerAscii->startTimeWhile();
            while((true) && (progSettings.timerTimeOut > timerAscii->endTimeWhile()))
            {
                if (progSettings.logicActive == digitalRead(handleMode))
                {
                    loopActivations++;
                    break;
                }
            }
            IAI->moveTo();
        }
        if (progSettings.offSurfaceProgSettings.boundaryPercentage >= ((float)loopActivations / (float)progSettings.offSurfaceProgSettings.loopCount))
        {
            boundaryLimit = true;
        }
        else
        {
            handleOffset = handleOffset - progSettings.offSurfaceProgSettings.searchPrecision;
            boundaryOffset = IAI->moveToMsg(handleOffset, 0.01, progSettings.currentSpeed, progSettings.currentAcceleration);
        }
        loopActivations = 0;
    }
    return handleOffset;
}

/**
 * @brief asciiProgram::programOne Handle testing program one.
 * Finds the handle using trigger seaking and tests number of times and at set speed.
 * emits sltDataToXML(data... etc.)
 */
void asciiProgram::programOne()
{
    QStringList loc;    // Location of the head at stop
    QStringList tme;    // Time that the travel took
    QStringList tst;    // Test index
    QStringList actualHandleLocation;   // Actual location of handle
    float handleLoc = 0;
    float speed = 150;
    float accel = 0.3;
    int t1ActivationCycles = 10;    // Number of activations to do in a row.
    int t1ActivationVal = 0;        // Current number of activations
    int t1TestCycles = 1;           // Number of cycles of the test to do in a row
    int t1TestCycleVal = 0;         // Current number of cycles

    if (CBRModeSelected == 0)
    {
        emit sigWarningMsg("Select Handle Mode Before Starting.");
    }
    else
    {
        IAI->servoON();  // Turn the IAI motor controller to on.
        timerAscii->waitMs(400); // Wait for it to settle
        IAI->HOME(); // Home the IAI actuator.
        while (t1TestCycleVal < t1TestCycles)   // loop that controls the number of cycles of the test.
        {
            IAI->moveTo();   // Move to a pre determined location every time... Using defaults.
            //---------------------------------------------------------------------------------------------------------------------//
            handleLoc = findHandleLocation();
            actualHandleLocation << QString::number(handleLoc * 100);
            //qDebug() << actualHandleLocation;
            IAI->isMoving();
            //---------------------------------------------------------------------------------------------------------------------//
            QByteArray GO_SPOT = IAI->moveToMsg((handleLoc - 0.1),0.01,speed,accel);
            //---------------------------------------------------------------------------------------------------------------------//
            CBRMode(false);
            while (t1ActivationVal < t1ActivationCycles)
            {
                gateInterupt = 0;
                cycVal = t1ActivationVal;
                IAI->gateCounter = 0;
                IAI->messageFrame(GO_SPOT);
                while (gateInterupt == 0)
                {
                    timerAscii->waitMs(1);
                }
                graphing = true;
                IAI->gateCounter = 0;
                timerAscii->startTimeWhile();
                timerAscii->startTime();
                while (true)
                {
                    if (digitalRead(HandleMODE) == 0)
                    {
                        tme << QString::number(timerAscii->endTime());
                        location = IAI->posNOW();
                        timerAscii->waitMs(60);
                        graphing = false;
                        t1ActivationVal++;
                        tst << QString::number(t1TestCycleVal);
                        break;
                    }
                    else if (timerAscii->endTimeWhile() > 40000)
                    {
                        location = IAI->posNOW();
                        timerAscii->waitMs(60);
                        t1ActivationVal++;
                        tme << "TimeOut";
                        tst << QString::number(t1TestCycleVal);
                        graphing = false;
                        break;
                    }
                }
                loc << QString::number(location);
                IAI->moveTo();
                timerAscii->waitMs(50);
            }
            IAI->HOME();
            t1ActivationVal = 0;
            t1TestCycleVal++;
            //qDebug() << t1TestCycleVal;
        }
        CBRMode(true);
        emit sigDataOut(loc, tme, tst, actualHandleLocation);
    }
    IAI->servoOFF();
}

/**
 * @brief asciiProgram::programTwo
 */
void asciiProgram::programTwo()
{
    bool lastTest = true;
    uint lastTestNum = 0;
    float sensorData;
    uint positionX = 0;
    uint positionY = 0;
    float handleLocation = 0; // location of handle surface from findHandleLocation().
    float fieldEdge = 0; // variable to hold field activations.
    uint gridWidth = 33; // total distance (mm) horizonally test will travel. nom = 33
    uint gridHeight = 27; // total distance (mm) vertically test will travel. nom = 27
    uint testPointSpacing = 1; // spacing between the points of data.
    uint testRep = 0; // Current test number at point.
    uint testRepTotal = 1; // total number of times test will run a a point. Then average.
    qRegisterMetaType<QVector<float>>("QVector<float>");
    qRegisterMetaType<QVector<uint>>("QVector<uint>");
    QVector<uint> twoDimArrayX;
    QVector<uint> twoDimArrayY;
    QVector<float> twoDimArrayBoundary;
    QVector<float> twoDimArraySurface;

    if (CBRModeSelected == 0)
    {
        emit sigWarningMsg("Select Handle Mode Before Starting.");
    }
    else
    {
        //--------Setup the IAI motor----------//
        IAI->servoON();  // Turn the IAI motor controller to on.
        timerAscii->waitMs(400); // Wait for it to settle
        IAI->HOME(); // Home the IAI actuator.
        //--------Setup the IAI motor----------//
        //--------Setup the stepper motor positions----------//
        if ((gridWidth % 2) == 0)
        {
            stepperControl(true, false, true, true, ((gridWidth/2) - 0.5f));    // offset the axis to center the test area over the original spot.
        }
        else
        {
            stepperControl(true, false, true, true, ((gridWidth - 1) / 2));     // Axis is a non even number so set center as a point.
        }
        timerAscii->waitMs(300);
        if ((gridHeight % 2) == 0)
        {
            stepperControl(false, true, false, false, ((gridHeight/2) - 0.5f));
        }
        else
        {
           stepperControl(false, true, false, false, ((gridHeight - 1) / 2));
        }
        timerAscii->waitMs(300);
        //--------Setup the stepper motor positions----------//
        //--------Test loop for Y----------//
        while(positionY < gridHeight)
        {
            positionX = 0;
            while (positionX < gridWidth)
            {
                handleLocation = findHandleLocation();  // find the surface of handle.
                timerAscii->waitMs(50);  // wait for PIC to clear out of handle function.
                CBRMode(false); // Set the handle mode selected on the GUI.
                timerAscii->waitMs(200); // Wait for I2C to complete and function to run.
                //----------- Test loop ------------//
                testRep = 0;
                while (testRep < testRepTotal)  // this added ability to do multiple tests for a single point.
                {
                    fieldEdge = handleLocation; // start search at handle surface.
                    sensorData = 0;
                    lastTest = true;
                    lastTestNum = 0;
                    while(lastTest)
                    {
                        IAI->moveTo();
                        timerAscii->waitMs(50);
                        IAI->isMoving();
                        IAI->messageFrame(IAI->moveToMsg(fieldEdge,0.01,150,1));
                        timerAscii->waitMs(50);
                        IAI->isMoving();
                        timerAscii->startTimeWhile();
                        while (timerAscii->endTimeWhile() < 400)
                        {
                            if (digitalRead(HandleMODE) == 0)
                            {
                                lastTest = true;
                                break;
                            }
                            else
                            {
                                lastTest = false;
                            }
                        }
                        fieldEdge = fieldEdge - 0.1;
                        lastTestNum++;
                    }
                    location = IAI->posNOW();
                    timerAscii->waitMs(60);
                    if (lastTestNum <= 1)
                    {
                        sensorData = handleLocation;
                    }
                    else
                    {
                        sensorData = location;
                    }
                    IAI->moveTo();
                    timerAscii->waitMs(50);
                    IAI->isMoving();
                    testRep++;
                }
                timerAscii->waitMs(50);
                CBRMode(true);
                timerAscii->waitMs(200);
                qDebug() << handleLocation << sensorData << handleLocation - sensorData;
                twoDimArrayBoundary << sensorData;
                twoDimArraySurface << handleLocation;
                twoDimArrayX << positionX;
                twoDimArrayY << positionY;
                timerAscii->waitMs(100);
                IAI->moveTo();
                positionX++;
                stepperControl(true, false, false, false, testPointSpacing);
            }
            timerAscii->waitMs(200);
            stepperControl(false, true, false, true, testPointSpacing);
            timerAscii->waitMs(200);
            stepperControl(true, false, true, true, gridWidth);
            positionY++;
            qDebug() << "NextRow";
        }
    }
    emit sigWriteXml3D(twoDimArraySurface, twoDimArrayBoundary, twoDimArrayX, twoDimArrayY, gridWidth, gridHeight);
    IAI->HOME();
    IAI->servoOFF();
}

/**
 * @brief asciiProgram::programThree
 */
void asciiProgram::programThree()
{
    //bool lastTest = true;
    float handleLocation = 0;

    IAI->servoON();  // Turn the IAI motor controller to on.
    timerAscii->waitMs(400); // Wait for it to settle
    IAI->HOME(); // Home the IAI actuator.
    handleLocation = findHandleLocation();
    CBRMode(false);

    while (1)
    {
        IAI->messageFrame(IAI->moveToMsg(handleLocation - 3));
        timerAscii->waitMs(200);
        IAI->isMoving();
        timerAscii->waitMs(400);
        IAI->moveTo();
        timerAscii->waitMs(50);
        IAI->isMoving();
    }

    CBRMode(true);
    IAI->servoOFF();

    /*
    while (lastTest)
    {
        chMsg = moveToMsg(handleLocation);
        messageFrame();
        timerAscii->waitMs(200);
        isMoving();
        timerAscii->startTimeWhile();
        while (timerAscii->endTimeWhile() < 400)
        {
            if (digitalRead(HandleMODE) == 0)
            {
                lastTest = true;
                break;
            }
            else
            {
                lastTest = false;
            }
        }
        moveTo();
        timerAscii->waitMs(50);
        isMoving();
        handleLocation = handleLocation - 0.1;
    }
    timerAscii->waitMs(5000);



    qRegisterMetaType<QVector<float>>("QVector<float>");
    qRegisterMetaType<QVector<uint>>("QVector<uint>");
    QVector<uint> twoDimArrayX;
    QVector<uint> twoDimArrayY;
    QVector<float> twoDimArrayBoundary;
    QVector<float> twoDimArraySurface;

    uint gridWidth = 33; // total distance (mm) horizonally test will travel. nom = 33
    uint gridHeight = 27; // total distance (mm) vertically test will travel. nom = 27

    for (uint i = 0; i < gridHeight; i++)
    {
        for (uint j = 0; j < gridWidth; j++)
        {
            twoDimArrayBoundary.append(i);
            twoDimArraySurface.append(i);
        }
    }

    emit sigWriteXml3D(twoDimArraySurface, twoDimArrayBoundary, twoDimArrayX, twoDimArrayY, gridWidth, gridHeight);
    */
}

void asciiProgram::threeDBoundaryProg(float handleSurface, controlSettings &progSettings)
{

}

void asciiProgram::boundarySearchAlgorithm()
{

}


void asciiProgram::fullTestProg(float handleSurface, controlSettings &progSettings)
{

}

void asciiProgram::responseTimeProg(float handleSurface, controlSettings &progSettings)
{

}

/**
 * @brief asciiProgram::manualModeControl
 */
void asciiProgram::manualModeControl()
{
    float handleLocation = 0;
    IAI->servoON();  // Turn the IAI motor controller to on.
    timerAscii->waitMs(400); // Wait for it to settle
    IAI->HOME(); // Home the IAI actuator.
    handleLocation = findHandleLocation();
    int cycleCount = 0;

    if (currentCycle == 0)
    {
        while (manualRun == true)
        {
            //qDebug() << "handleLocation" << handleLocation << "currentOffset" << currentOffset << "currentPositioning" << currentPositioning << "currentSpeed" << currentSpeed << "currentAcceleration" << currentAcceleration;
            IAI->messageFrame(IAI->moveToMsg(handleLocation - currentOffset, currentPositioning, currentSpeed, currentAcceleration));
            timerAscii->waitMs(200);
            IAI->isMoving();
            timerAscii->waitMs(400);
            IAI->moveTo();
            timerAscii->waitMs(50);
            IAI->isMoving();
        }
    }
    else
    {
        while ((currentCycle > cycleCount) && (manualRun == true))
        {
            //qDebug() << "handleLocation" << handleLocation << "currentOffset" << currentOffset << "currentPositioning" << currentPositioning << "currentSpeed" << currentSpeed << "currentAcceleration" << currentAcceleration;
            IAI->messageFrame(IAI->moveToMsg(handleLocation - currentOffset, currentPositioning, currentSpeed, currentAcceleration));
            timerAscii->waitMs(200);
            IAI->isMoving();
            timerAscii->waitMs(400);
            IAI->moveTo();
            timerAscii->waitMs(50);
            IAI->isMoving();
            cycleCount++;
        }
    }

    IAI->servoOFF();
}

/**
 * @brief asciiProgram::stepperHome
 */
void asciiProgram::stepperHome()
{
    uint8_t command = 0x03;
    IIC->writeData_I2C(1,command,0b00000101,0x00);
    timerAscii->waitMs(10);
    command = 0x1;
    IIC->writeData_I2C(1,command,0x00,0x00);
    timerAscii->waitMs(10);
    timerAscii->startTimeWhile();
    while (stp_move_done == false && timerAscii->endTimeWhile() < 200000)
    {

    }
}

/**
 * @brief asciiProgram::sltStepperJogPos
 * @param stepperX
 * @param stepperY
 */
void asciiProgram::sltStepperJogPos(bool stepperX, bool stepperY)
{
    stepperControl(stepperX, stepperY, false, true, 0.5);
}

/**
 * @brief asciiProgram::sltStepperJogNeg
 * @param stepperX
 * @param stepperY
 */
void asciiProgram::sltStepperJogNeg(bool stepperX, bool stepperY)
{
    stepperControl(stepperX, stepperY, true, false, 0.5);
}

/**
 * @brief asciiProgram::stepperControl
 * @param motorX
 * @param motorY
 * @param left
 * @param up
 * @param mm
 */
void asciiProgram::stepperControl(bool motorX, bool motorY, bool left, bool up, float mm)
{
    uint8_t command;
    // command code 0x03 is the stepper parameters code.
    command = 0x03;
    uint8_t dataH, dataL;
    dataH = 0b00000101;
    dataL = 0;
    if (motorX)
    {
        // enable motor 1 horizontal
        dataH = (dataH | 0b01000000);
    }
    if (motorY)
    {
        // enable motor 2 vertical
        dataH = (dataH | 0b10000000);
    }
    if (left)
    {
        dataH = (dataH | 0b00010000);
    }
    if (up)
    {
        dataH = (dataH | 0b00100000);
    }
    IIC->writeData_I2C(1,command,dataH,dataL);
    timerAscii->waitMs(10);
    // command 0x02 is the start move and how many steps to take command
    command = 0x02;
    uint16_t steps;
    // 800 steps is 1 mm.
    // This only holds true at 1/4 step microstepping
    steps = (mm * 800);
    dataL = steps;
    dataH = steps >> 8;
    IIC->writeData_I2C(1,command, dataH, dataL);
    timerAscii->waitMs(10);

    timerAscii->startTimeWhile();
    while (stp_move_done == false && timerAscii->endTimeWhile() < 200000)
    {

    }
}

/**
 * @brief asciiProgram::sltMotionSelection
 * @param motor
 */
void asciiProgram::sltMotionSelection(QString motor)
{
   MotorSelected = motor;
}

/**
 * @brief asciiProgram::sltCBRModeChange
 * @param mode
 */
void asciiProgram::sltCBRModeChange(uint mode)
{
    CBRModeSelected = mode;
}

/**
 * @brief asciiProgram::sltCBRManualChange
 * @param mode
 */
void asciiProgram::sltCBRManualChange(uint mode)
{
    uint8_t commandCode = 0x0E;
    uint8_t dataH;
    uint8_t dataL = 0x5A;

    if (mode == 0)
    {
        dataH = 0;
        dataL = 0xA5;
    }
    else
    {
        dataH = mode;
    }

    IIC->writeData_I2C(IIC->cbrSlaveAddress,commandCode, dataH, dataL);
    //qDebug() << "manual" << mode;
}

/**
 * @brief asciiProgram::CBRMode
 * @param mode
 */
void asciiProgram::CBRMode(bool erase)
{
    uint8_t commandCode = 0x0E;
    uint8_t dataH;
    uint8_t dataL = 0x5A;

    if (CBRModeSelected == 0)
    {
        dataH = 0;
        dataL = 0xA5;
    }
    else
    {
        dataH = CBRModeSelected;
    }

    if (erase == true)
    {
        dataH = 0;
        dataL = 0xA5;
    }

    IIC->writeData_I2C(IIC->cbrSlaveAddress,commandCode, dataH, dataL);
}
			
/**
 * @brief asciiProgram::myInterrupt
 */
void asciiProgram::myInterrupt()
{
    gateInterupt = 1;
}

/**
 * @brief asciiProgram::myInterrupt2
 */
void asciiProgram::myInterrupt2()
{
    if (stp_move_done == true)
    {
        stp_move_done = false;
    }
    else
    {
        stp_move_done = true;
    }
}

/**
 * @brief asciiProgram::graphOut
 * @param speedGraph

void asciiProgram::graphOut(double speedGraph[50][50])
{
	int fileNum = 0;
	QDir dir;
	QDir::setCurrent("/media/pi/");
	QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
	if (!list.empty())
	{
		QString usb = list.first().absoluteFilePath();
		QDir::setCurrent(usb);
	//qDebug() << usb;
		if (!dir.cd(usb))
			qWarning("Cannot find the USB directory");
		else
		{
			QXmlStreamWriter xmlWriter;
			xmlWriter.setAutoFormatting(true);
			QFile file(QDate::currentDate().toString("MM-dd-yy") + "GraphData" + QString::number(fileNum) + ".xml");
			//QDir::setCurrent(dir);
			//qDebug() << file.fileName() << "exists:" << file.exists();
			while (file.exists())
			{
				fileNum++;
				file.setFileName(QDate::currentDate().toString("MM-dd-yy") + "GraphData" + QString::number(fileNum) + ".xml");
			}
			//qDebug() << QDir::currentPath() << file.fileName() << "fileNum" << fileNum;
			if (!file.open(QIODevice::WriteOnly))
			{
				qDebug() << "File did not open!";
			}
			else
			{
                // Set the device to be written to as the QFile created above.
				xmlWriter.setDevice(&file);
                // Writes the standard XML start header with version and character type used.
				xmlWriter.writeStartDocument();
				xmlWriter.writeStartElement("TestResults");
                // Start Element-------------------------------------------------------------------------------------------------
				for (int i = 0; i < 50; i++)
				{
					for (int j = 0; j < 50; j++)
					{
						//qDebug() << i << j << speedGraph[i][j];
						if (speedGraph[i][j] == 0.0 && j != 0)
							break;
						xmlWriter.writeStartElement("GraphingResults");
						xmlWriter.writeTextElement("GraphResults", QString::number(speedGraph[i][j]));
						xmlWriter.writeEndElement();
					}
				}
                // End Element---------------------------------------------------------------------------------------------------
				xmlWriter.writeEndElement();
				xmlWriter.writeEndDocument();
				fileNum++;
			}
		}
	} 
}

*/
