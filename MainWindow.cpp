/** \file MainWindow.cpp
*	Author: Trenton Wells
*
*	Created: July 29, 2016, 3:27 PM
*/

#include "MainWindow.h"

/** \brief MainWindow constructor.
*	Qt parent is created to pass events to Ui.
*	Ui handle is created with handle 'Ui'.
*	Calls the start method of the QThread base class. 
*/
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	prog->start(QThread::TimeCriticalPriority);
    setSettings();
}

/** \brief MainWindow destructor. 
*	Releases the Ui handle back to memory for the system.
*/
MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::servo_toggled()
{
	if (ui->btnServo->isChecked())
	{
        emit sigIAIServoOnOff(1);
		ui->btnServo->setText("Servo OFF");
	}
	else
	{
        emit sigIAIServoOnOff(0);
		ui->btnServo->setText("Servo ON");
	}
	
}

/** \brief MainWindow alarmClear event handler.
*	Handles the MainWindow Ui alarmClear button clicked signal.
*/
void MainWindow::alarmClear_clicked()
{

}

/** \brief MainWindow cancelMove event handler.
*	Handles the MainWindow Ui cancelMove button clicked signal.
*/
void MainWindow::cancelMove_clicked()
{
	emit sigWindowValRunning(false);
}

/** \brief MainWindow btnHOME event handler.
*	Handles the MainWindow Ui btnHOME button clicked signal.
*/
void MainWindow::HOME_clicked()
{
    if (ui->rbtnIAI->isChecked())
    {
        emit sigIAIHome();
    }
    else
    {
        emit sigStepperHome();
    }
}

/** \brief MainWindow posNOW event handler.
*	Handles the MainWindow Ui posNOW button clicked signal.
*/
void MainWindow::posNOW_clicked()
{

}

/** \brief MainWindow incLarge event handler.
*	Handles the MainWindow Ui incLarge button clicked signal.
*/
void MainWindow::incLarge_clicked()
{

}

/** \brief MainWindow incSmall event handler.
*	Handles the MainWindow Ui incSmall button clicked signal.
*/
void MainWindow::incSmall_clicked()
{

}

/** \brief MainWindow jogNeg event handler.
*	Handles the MainWindow Ui jogNeg button pressed signal.
*	This signal is sent as soon as the Ui button is hit.
*/
void MainWindow::jogNeg_clicked()
{
    if (ui->rbtnIAI->isChecked())
    {
        emit sigIAIJogNeg();
    }
    else if (ui->rbtnStepper1->isChecked())
    {
        emit sigStepperJogNeg(true, false);
    }
    else
    {
        emit sigStepperJogNeg(false, true);
    }
}

/** \brief MainWindow jogNeg event handler.
*	Handles the MainWindow Ui jogNeg button released signal.
*	This signal is sent as soon as the Ui button is released.
*/
void MainWindow::jogNeg_released()
{
    emit sigIAIJogNegStop();
}

/** \brief MainWindow jogPos event handler.
*	Handles the MainWindow Ui jogPos button pressed signal.
*	This signalissent ass soon as the Ui button is hit.
*/
void MainWindow::jogPos_clicked()
{
    if (ui->rbtnIAI->isChecked())
    {
        emit sigIAIJogPos();
    }
    else if (ui->rbtnStepper1->isChecked())
    {
        emit sigStepperJogPos(true, false);
    }
    else
    {
        emit sigStepperJogPos(false, true);
    }
}

/** \brief MainWindow jogPos event handler.
*	Handles the MainWindow Ui jogPos button pressed signal.
*	This signalissent as soon as the Ui button is released.
*/
void MainWindow::jogPos_released()
{
    emit sigIAIJogPosStop();
}

/** \brief MainWindow method to update text.
*	Updates the text lables on the MainWindow Ui to read the Hex ascii commands sent and recieved.
*	Does not currently work with threads.
*/

void MainWindow::manualControl()
{
    if (ui->btnMaunRun->isChecked())
    {
        emit sigManuValsControl(true);
        ui->btnMaunRun->setText("Manual Stop");
    }
    else
    {
        emit sigManuValsControl(false);
        ui->btnMaunRun->setText("Manual Run");
    }
}

void MainWindow::setSettings()
{
    CBRMode << "Select Handle Mode" << "Conti-Ford" << "Conti-Chrysler" << "Hella" << "Discrete" << "Volvo" << "INFO" << "ADAC";

    for (int i = 0; i < 31; i++)
	{
        target << (i * 5);
    }
	for (int i = 1; i < 251; i++)
	{
        speed << i;
	}

    positioning << 0.01 << 0.05 << 0.1 << 0.2 << 0.5 << 1 << 2 << 3 << 4 << 5 << 10 << 20 << 30 << 40 << 50;
    acceleration << 0.1 << 0.2 << 0.3 << 0.4 << 0.5 << 0.6 << 0.7 << 0.8 << 0.9 << 1.0;
    push << 20 << 30 << 40 << 50 << 60 << 70;
	ctlrFlag << "Normal" << "Push" << "Incremental";
    float counter = 0;
    for (int i = 0; i < 101; i++)
    {
        offset << counter;
        counter = counter + 0.1;
    }
    cycle << 0 << 5 << 10 << 20 << 50 << 100 << 150 << 200;

    ui->cboxCBRManual->addItems(CBRMode << "Clear");
    ui->cboxCBRMode->addItems(CBRMode);

    for (int a = 0; a < target.length(); a++)
    {
        QString num;
        num = QString::number(target.at(a));
        ui->cboxTarget->addItem(QString::number(target.at(a)));
    }
    for (int b = 0; b < positioning.length(); b++)
    {
        ui->cboxPosBand->addItem(QString::number(positioning.at(b)));
    }
    for (int c = 0; c < speed.length(); c++)
    {
        ui->cboxSpeed->addItem(QString::number(speed.at(c)));
    }
    for (int d = 0; d < positioning.length(); d++)
    {
        ui->cboxZonePos->addItem(QString::number(positioning.at(d)));
        ui->cboxZoneNeg->addItem(QString::number(positioning.at(d)));
    }
    for (int e = 0; e < acceleration.length(); e++)
    {
        ui->cboxAcceleration->addItem(QString::number(acceleration.at(e)));
        ui->cboxDeceleration->addItem(QString::number(acceleration.at(e)));
    }
    for (int f = 0; f < push.length(); f++)
    {
        ui->cboxPush->addItem(QString::number(push.at(f)));
    }

	ui->cboxMoveMode->addItems(ctlrFlag);

    for (int g = 0; g < offset.length(); g++)
    {
        ui->cboxOffset->addItem(QString::number(offset.at(g)));
    }
    for (int h = 0; h < cycle.length(); h++)
    {
        ui->cboxCycle->addItem(QString::number(cycle.at(h)));
    }

    btnDefaults_clicked();
    settingsIndex_changed();
}

void MainWindow::btnResponseTime_clicked()
{
    emit sigResponseTimeProg();
}

void MainWindow::btn3DBoundary_clicked()
{
    prog->programControlSettings.currentTarget = 1;
    emit sigBoundryProg(prog->programControlSettings);
}

void MainWindow::btnFullTest_clicked()
{
    emit sigFullTestProg();
}

void MainWindow::moveTo_clicked()
{
	emit sigWindowValCase(17);
}

void MainWindow::btnDefaults_clicked()
{
	ui->cboxTarget->setCurrentIndex(0);
	ui->cboxPosBand->setCurrentIndex(0);
	ui->cboxSpeed->setCurrentIndex(149);
    ui->cboxZonePos->setCurrentIndex(0);
    ui->cboxZoneNeg->setCurrentIndex(0);
    ui->cboxAcceleration->setCurrentIndex(0);
    ui->cboxDeceleration->setCurrentIndex(0);
    ui->cboxPush->setCurrentIndex(0);
	ui->cboxMoveMode->setCurrentIndex(0);
    ui->cboxOffset->setCurrentIndex(0);
    ui->cboxCycle->setCurrentIndex(0);
}

void MainWindow::settingsIndex_changed()
{
    currentTarget = target.at(ui->cboxTarget->currentIndex());
    currentPositioning = positioning.at(ui->cboxPosBand->currentIndex());
    currentSpeed = speed.at(ui->cboxSpeed->currentIndex());
    currentZonePos = positioning.at(ui->cboxZonePos->currentIndex());
    currentZoneNeg = positioning.at(ui->cboxZoneNeg->currentIndex());
    currentAcceleration = acceleration.at(ui->cboxAcceleration->currentIndex());
    currentDeacceleration = acceleration.at(ui->cboxDeceleration->currentIndex());
    currentPush = push.at(ui->cboxPush->currentIndex());
	currentCtlrFlag = ui->cboxMoveMode->currentText();
    currentOffset = offset.at(ui->cboxOffset->currentIndex());
    currentCycle = cycle.at(ui->cboxCycle->currentIndex());
    emit sigManuVals(currentTarget, currentPositioning, currentSpeed, currentZonePos, currentZoneNeg, currentAcceleration, currentDeacceleration, currentPush, currentCtlrFlag, currentOffset, currentCycle);
}

void MainWindow::writeXml_clicked()
{
	emit sigWriteXml();
}

void MainWindow::lock_clicked()
{
	QString str = ui->btnLOCK->text();
	if (str == "LOCK")
	{
		ui->btnLOCK->setText("UNLOCK");
        str = "UNLOCK";
	}
	else
	{
		ui->btnLOCK->setText("LOCK");
        str = "LOCK";
	}
	emit sigHandleMODE(str);
}

void MainWindow::CBR_Mode_Changed()
{
    uint index = ui->cboxCBRMode->currentIndex();
    emit sigCBRModeChange(index);
}

void MainWindow::CBR_Manual_Changed()
{
    uint index = ui->cboxCBRManual->currentIndex();
    emit sigCBRManualChange(index);
}

void MainWindow::motionSelection_changed()
{
    if (ui->rbtnIAI->isChecked())
    {
        emit sigMotionSelection("IAI");
    }
    else if (ui->rbtnStepper1->isChecked())
    {
        emit sigMotionSelection("STP1");
    }
    else if (ui->rbtnStepper2->isChecked())
    {
        emit sigMotionSelection("STP2");
    }
}

void MainWindow::sltWarningMsg(QString text)
{
    QMessageBox warning;
    warning.setText(text);
    warning.exec();
}

void MainWindow::btnOffSurface_clicked()
{
    //ui->statusBar->showMessage("msg", 10000);
    emit sigOffSurfaceProg();
}

void MainWindow::sltStatusMsg(QString msg, int timeOut)
{
    ui->statusBar->showMessage(msg, timeOut);
}





