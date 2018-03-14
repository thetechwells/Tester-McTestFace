/** \file asciiProgram.h
*	Author: Trenton Wells
*
*	Created: July 29, 2016, 3:28 PM
*/

#pragma once

#include <QtCore>
#include <QString>
#include <QThread>
#include <QDebug>

#include "timeManager.h"
#include "serialPortManager.h"
#include "wiringPi.h"
#include "wiringPiI2C.h"
#include "i2c.h"
#include "actuator.h"
#include "stepper.h"
#include "dataToXml.h"


using namespace std;

/**
 * @brief The asciiProgram class Secondary thread for program control.
 *	Main program that recieves signals from the UI and runs main functions for the controller.
 */
class asciiProgram : public QThread
{	
	Q_OBJECT
		
public:
	explicit asciiProgram(QObject *parent = 0);
	void run();
	~asciiProgram();
	static void myInterrupt();
    static void myInterrupt2();
	static double speedGraph[50][50];
	static QElapsedTimer cslul;
	static int cycVal;
    void writeData_I2C(uint8_t commandCode, uint8_t data0, uint8_t data1);
    void stepperControl(bool motorX, bool motorY, bool left, bool up, float mm);
    void stepperHome(void);
    uint16_t readData_I2C(uint8_t commandCode);
    QObject * parentProgram;

    struct responseTimeControlSettings
    {

    };

    struct threeDBoundaryControlSettings
    {
        int mmX;
        int mmY;
        int centerPoint;
    };

    struct offSurfaceControlSettings
    {
        int loopCount;
        float maxBoundary;
        float minBoundary;
        float searchPrecision;
        float boundaryPercentage;
    };

    struct controlSettings
    {
        QString programName;
        uint CBRModeVal;
        float currentTarget;
        float currentPositioning;
        float currentSpeed;
        float currentZonePos;
        float currentZoneNeg;
        float currentAcceleration;
        float currentDeacceleration;
        float currentPush;
        QString currentCtlrFlag;
        float currentOffset;
        int currentCycle;
        int timerTimeOut = 5000;
        int logicActive = 0;
        offSurfaceControlSettings offSurfaceProgSettings;
        responseTimeControlSettings responseTimeProgSettings;
        threeDBoundaryControlSettings threeDBoundaryProgSettings;
    };
    controlSettings programControlSettings;

signals:
	void sigCompDone(float);
	void sigDataOut(QStringList, QStringList, QStringList, QStringList);
	void sigWatch();
    void sigWarningMsg(QString);
    void sigWriteXml3D(QVector<float>,QVector<float>, QVector<uint>, QVector<uint>, uint, uint);
    void sigStatusMsg(QString,int);
	
public slots:
	void sltHandleMODE(QString);
	void sltValRunning(bool);
    void sltManuVals(float, float, float, float, float, float, float, float, QString, float, int);
    void sltManuValsControl(bool);
    void sltMotionSelection(QString);
    void sltCBRModeChange(uint);
    void sltCBRManualChange(uint);
    void sltStepperJogNeg(bool, bool);
    void sltStepperJogPos(bool, bool);
    void sltRunProg(controlSettings &progSettings);
	
private:
    serialPortManager *COM;
    actuator *IAI;
    i2c *IIC;
    stepper *STP;
    timeManager *timerAscii;
	QTimer stepTimer;
    dataToXml *DTX = new dataToXml();

#define LOCK 7
#define UNLOCK 8
#define GATE_PIN 24
#define MOTION_DONE 23
#define FORWARD 1
#define BACKWARD 0
	
    float currentTarget = 0;
    float currentPositioning = 0;
    float currentSpeed = 0;
    float currentZonePos = 0;
    float currentZoneNeg = 0;
    float currentAcceleration = 0;
    float currentDeacceleration = 0;
    float currentPush = 0;
    QString currentCtlrFlag = 0;
    float currentOffset = 0;
    int currentCycle = 0;
	bool running;
    bool manualRun;	
    unsigned int SLAVE_ADDRESS;
    QString IAI_SLAVE_ADDRESS = "01";   // IAI slave address for RS-485.
    QString IAI_CODE_READ = "03";       // Read data and status.
    QString STP1 = "ST1";
    QString STP2 = "ST2";
    QString MotorSelected = "IAI";
	int HandleMODE;
    int CBRModeSelected = 0;
    int CBRHandleMode = 0x5A;
	float location = 0;
	
    float findHandleLocation();
    void threeDBoundaryProg(float handleSurface, controlSettings &progSettings);
    void boundarySearchAlgorithm();
    void fullTestProg(float handleSurface, controlSettings &progSettings);
    float offSurfaceProg(float handleSurface, controlSettings &progSettings);
    void responseTimeProg(float handleSurface, controlSettings &progSettings);
    void programOne();
	void programTwo();
	void programThree();
    void manualModeControl();
    void CBRMode(bool erase);
};

