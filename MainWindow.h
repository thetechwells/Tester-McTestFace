/** \file MainWindow.h
*	Author: Trenton Wells
*
*	Created: July 29, 2016, 3:25 PM
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QString>
#include "ui_MainWindow.h"
#include "asciiProgram.h"
#include "dataToXml.h"

namespace Ui 
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
    
public :
	explicit MainWindow(QWidget *parent = 0); /** MainWindow Constructor. */
	~MainWindow(); /** MainWindow destructor. */
	asciiProgram *prog = new asciiProgram(this);
	
signals:
	void sigWindowValCase(int);
	void sigWindowValRunning(bool);
	void sigWriteXml();
    void sigManuVals(float, float, float, float, float, float, float, float, QString, float, int);
    void sigManuValsControl(bool);
    void sigHandleMODE(QString);
    void sigCBRModeChange(uint);
    void sigCBRManualChange(uint);
    void sigMotionSelection(QString);

    void sigStepperHome();
    void sigStepperJogPos(bool, bool);
    void sigStepperJogNeg(bool, bool);
    void sigIAIServoOnOff(int);
    void sigIAIHome();
    void sigIAIJogPos();
    void sigIAIJogPosStop();
    void sigIAIJogNeg();
    void sigIAIJogNegStop();
    void sigResponseTimeProg();
    void sigBoundryProg(asciiProgram::controlSettings);
    void sigOffSurfaceProg();
    void sigFullTestProg();
	
public slots :
    void sltWarningMsg(QString);
    void sltStatusMsg(QString, int timeOut = 10000);

protected slots :
	void servo_toggled();					/** Connected to the servo ON/OFF button on MainWindow. */
	void alarmClear_clicked();				/** Connected to the alarmClear button on MainWindow. */
	void cancelMove_clicked();				/** Connected to the cancelMove button on MainWindow. */
	void HOME_clicked();					/** Connected to the btnHOME button on MainWindow. */
	void posNOW_clicked();					/** Connected to the posNOW button on MainWindow. */
	void incLarge_clicked();				/** Connected to the incLarge button on MainWindow. */
	void incSmall_clicked();				/** Connected to the incSmall button on MainWindow. */
	void jogNeg_clicked();					/** Connected to the jogNeg button on MainWindow. */
	void jogNeg_released();					/** Connected to the jogNeg button on MainWindow. */
	void jogPos_clicked();					/** Connected to the jogPos button on MainWindow. */
	void jogPos_released();					/** Connected to the jogPos button on MainWindow. */
	void lock_clicked();					/** Connected to the btnLock button on MainWindow tab Manual Mode. */
    void btnResponseTime_clicked();			/** Connected to the button on MainWindow tab Program. */
    void btn3DBoundary_clicked();			/** Connected to the button on MainWindow tab Program. */
    void btnOffSurface_clicked();
    void btnFullTest_clicked();             /** Connected to the button on MainWindow tab Program. */
	void moveTo_clicked();					/** Connected to the btnMove button on MainWindow tab Manual Mode. */
	void btnDefaults_clicked();				/** Connected to the btnDefaults button on MainWindow tab Settings. */
    void manualControl();
	void settingsIndex_changed();			/** Connected to the btnWatch button on MainWindow tab Manual Mode. */
	void writeXml_clicked();				/** Connected to the btnWriteXML button on MainWindow tab Manual Mode. */
    void CBR_Mode_Changed();                /** Connected to the cboxCBRMode combo box on MainWindow */
    void CBR_Manual_Changed();
    void motionSelection_changed();
	
private:
	Ui::MainWindow *ui;
	void setSettings();
	float location = 0;
    QStringList CBRMode;

    QList<float> target;
    QList<float> positioning;
    QList<float> speed;
    QList<float> acceleration;
    QList<float> push;
    QStringList ctlrFlag;
    QList<float> offset;
    QList<int> cycle;

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

};

#endif // MAINWINDOW_H
