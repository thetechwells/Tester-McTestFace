/** \file dataToXml.h
*	Author: Trenton Wells
*
*	Created: August 15, 2016, 8:55 AM
*/

#pragma once

#include <QtCore>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QWidget>

class dataToXml : public QThread
{
	Q_OBJECT
		
public:
	dataToXml();
	~dataToXml();
	void run();
	
signals:
	
public slots :
    void sltWriteXml3D(QVector<float>,QVector<float>, QVector<uint>, QVector<uint>, uint, uint);
	void sltDataIn(QStringList, QStringList, QStringList, QStringList);
	
private:
    void writeXMLResponseTime();
    void writeXML3D();
	void doCalc();
	
    QVector<float> dataArray1;
    QVector<float> dataArray2;
    QVector<uint> widthArray;
    QVector<uint> heightArray;
    uint width3D;
    uint height3D;
	QStringList location;
	QStringList time;
	QStringList responseTime;
	QStringList testNum;
	QStringList actualLocation;
	float totMoveTime;
	int fileNum = 0;
	int calls = 0;
	QFileDialog dialog;
	QString dir;

    QStringList surfaceX;
    QStringList surfaceY;
    QStringList surfaceZ;
    QStringList boundryX;
    QStringList boundryY;
    QStringList boundryZ;
};

