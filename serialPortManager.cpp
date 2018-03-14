/** \file serialPortManager.cpp
*	Author: Trenton Wells
*
*	Created: July 29, 2016, 3:30 PM
*/

#include "serialPortManager.h"

serialPortManager::serialPortManager()
{

}

serialPortManager::~serialPortManager()
{
    serial->close();
    delete serial;
}

void serialPortManager::run()
{
    timer = new timeManager();
    serial = new QSerialPort();

    QList<QSerialPortInfo> systemPorts;
    QString portName;

    systemPorts = QSerialPortInfo::availablePorts();	/* Qt class collects all of the avaliable serial ports on the raspberry pi. */

    for (int i = 0; i < systemPorts.size(); i++)
    {
        if (systemPorts.at(i).manufacturer() == "FTDI")	/* Reading the manufacturer name to determin the serial rs485 device. */
        {
            portName = systemPorts.at(i).portName();
        }
    }
    serial->setPortName(portName);
    serial->setBaudRate(baudRate);
    serial->setDataBits(QSerialPort::Data8);
    serial->open(QIODevice::ReadWrite);
    //this->exec();
}

void serialPortManager::sltSerialMsgIn(QByteArray Tx)
{
    if (serial->isOpen())
    {
        serial->write(Tx);		// Write the QByteArray to the serail port stream.
        serial->waitForBytesWritten(waitTimeoutWrite);

        QByteArray data;	// Inisilize the return QByteArray.

        serial->waitForReadyRead(waitTimeoutRead);
        timer->startTimeWhile();
        while (1)
        {
            data.append(serial->readLine());
            if (data.endsWith('\n') || timer->endTimeWhile() > 64)
            {
                break;
            }
            serial->waitForReadyRead(waitTimeoutRead);
        }
        emit sigSerialMsgOut(data);
    }
}
