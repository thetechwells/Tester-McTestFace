#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <QObject>
#include <QDebug>
#include "timeManager.h"

class actuator : public QObject
{
    Q_OBJECT
public:
    explicit actuator(QObject *parent = nullptr);
    ~actuator();

    void messageFrame(QByteArray);
    QByteArray LRC(QByteArray);
    void MODBUS();
    void alarmClr();
    float posNOW();
    void moveInc();
    void moveJogPos();
    void moveJogPosStop();
    void moveJogNeg();
    void moveJogNegStop();
    void moveStop();
    void isMoving();
    void readReg9007();
    void servoON();
    void servoOFF();
    void HOME();

    void moveTo(float target = 0, float posBand = 0.01, float speed = 150, float acceleration = 0.3);
    QByteArray pushToMsg(QString slaveAddress = "01", float target = 10, float band = 140, float speed = 30, float accel = 0.3, float pushStrenght = 0.20);
    QByteArray moveToMsg(float target = 0, float posBand = 0.01, float speed = 150, float acceleration = 0.3);
    QByteArray pushToHex(float);
    QByteArray accelToHex(float);
    QByteArray targetPosToHex(float);
    QByteArray posBandToHex(float);
    QByteArray speedToHex(float);
    QByteArray controlToHex(bool, bool, bool);
    QByteArray dataReturn;

    int gateCounter = 0;

private:
    timeManager *actuatorTimer;
    struct ERCmessage
    {
        QByteArray msg = "";
        QByteArray lastMoveMsg = "";
    } lastMsg;

    QString IAI_CODE_POS_WRITE = "10";  // Write position data and numerical values.
    QString IAI_CODE_READ = "03";       // Read data and status.
    QString IAI_CODE_OPC_WRITE = "05";  // Write to operation command functions.
    QString IAI_CODE_CON_WRITE = "06";  // Write to control functions.
    qint8 funCode;
    qint8 numOfBytes;

    #define FORWARD 1
    #define BACKWARD 0

signals:
    void sigMsgOut(QByteArray);

public slots:
    void sltMsgIn(QByteArray);
    void sltServoOnOff(int);
    void sltHome();
};

#endif // ACTUATOR_H
