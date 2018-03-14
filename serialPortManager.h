/** \file serialPortManager.h
*	Author: Trenton Wells
*
*	Created: July 29, 2016, 3:30 PM
*/

#pragma once

#include "timeManager.h"

#include <QApplication>
#include <QtSerialPort>
#include <QThread>
#include <QDebug>

class serialPortManager : public QThread
{
	Q_OBJECT
		
public:
    explicit serialPortManager();	/** Constructor. */
	~serialPortManager();	/** Destructor. */
	
signals:
	void error(const QString &s);
	void sigSerialMsgOut(QByteArray);
	
public slots:
	void sltSerialMsgIn(QByteArray Tx);

protected:
    void run();
	
private:
    QSerialPort *serial;
    timeManager *timer;
	int waitTimeoutRead = 2;
	int waitTimeoutWrite = 300;
	qint32 baudRate = 38400;
};
