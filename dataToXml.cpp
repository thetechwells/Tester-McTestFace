/** \file dataToXml.cpp
*	Author: Trenton Wells
*
*	Created: August 15, 2016, 8:55 AM
*/

#include "dataToXml.h"

dataToXml::dataToXml()
{

}

dataToXml::~dataToXml()
{
}

/** \brief Overloaded thread function.
*/
void dataToXml::run()
{
	
}

/** \brief XML document builder. 
*	Opens new file and saves the current data to the file.
*	Using Qt XML creator to manage the XML parsing.
*/
void dataToXml::writeXMLResponseTime()
{
	
	QXmlStreamWriter xmlWriter;
	xmlWriter.setAutoFormatting(true);
	
    QFile file(QDate::currentDate().toString("yy-MM-dd") + "_ResponseData_" + QString::number(fileNum) + ".xml");
	
	QDir::setCurrent(dir);

	while (file.exists())
	{
		fileNum++;
        file.setFileName(QDate::currentDate().toString("yy-MM-dd") + "_ResponseData_" + QString::number(fileNum) + ".xml");
	}
	
	if (!file.open(QIODevice::WriteOnly))
	{
		qDebug() << "File did not open!";
	}
	else
	{
		/* Set the device to be written to as the QFile created above. */
		xmlWriter.setDevice(&file);
		
		/* Writes the standard XML start header with version and character type used. */
		xmlWriter.writeStartDocument();
		
		xmlWriter.writeStartElement("TestResults");
		
		/* Start Element------------------------------------------------------------------------------------------------- */
		for (int i = 0; i < testNum.length(); i++)
		{
			xmlWriter.writeStartElement("ResponseTimeResults");
			xmlWriter.writeTextElement("DataTestSet", testNum.at(i));
			xmlWriter.writeTextElement("SampleNumber", QString::number(i + 1));
			xmlWriter.writeTextElement("RawLocation", location.at(i));
			xmlWriter.writeTextElement("RawTime", time.at(i));
			xmlWriter.writeTextElement("CalculatedResponseTime", responseTime.at(i));
			xmlWriter.writeEndElement();
		}
		/* End Element--------------------------------------------------------------------------------------------------- */

		xmlWriter.writeEndElement();
		
		xmlWriter.writeEndDocument();
		
		fileNum++;
	}
}

void dataToXml::writeXML3D()
{
    QXmlStreamWriter xmlWriter;
    xmlWriter.setAutoFormatting(true);
    QFile file(QDate::currentDate().toString("yy-MM-dd") + "_3DData_" + QString::number(fileNum) + ".xml");
    QDir::setCurrent(dir);

    while (file.exists())
    {
        fileNum++;
        file.setFileName(QDate::currentDate().toString("yy-MM-dd") + "_3DData_" + QString::number(fileNum) + ".xml");
    }

    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "File did not open!";
    }
    else
    {
        /* Set the device to be written to as the QFile created above. */
        xmlWriter.setDevice(&file);
        /* Writes the standard XML start header with version and character type used. */
        xmlWriter.writeStartDocument();
        xmlWriter.writeStartElement("TestResults");

        int num = 0;

        /* Start Element------------------------------------------------------------------------------------------------- */
        xmlWriter.writeStartElement("BoundryResults");
        for (uint j = 0; j < (height3D); j++)
        {
            xmlWriter.writeStartElement("row");
            for (uint i = 0; i < (width3D); i++)
            {
                xmlWriter.writeTextElement("X" + QString::number(i) + "Subtraction", QString::number(dataArray1.at(num) - dataArray2.at(num)));
                num++;
            }
            xmlWriter.writeEndElement();
        }
        num = 0;
        for (uint j = 0; j < (height3D); j++)
        {
            xmlWriter.writeStartElement("row");
            for (uint i = 0; i < (width3D); i++)
            {
                xmlWriter.writeTextElement("X" + QString::number(i) + "surfaceZ", QString::number(dataArray1.at(num)));
                num++;
            }
            xmlWriter.writeEndElement();
        }
        num = 0;
        for (uint j = 0; j < (height3D); j++)
        {
            xmlWriter.writeStartElement("row");
            for (uint i = 0; i < (width3D); i++)
            {
                xmlWriter.writeTextElement("X" + QString::number(i) + "boundryZ", QString::number(dataArray2.at(num)));
                num++;
            }
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
        /* End Element--------------------------------------------------------------------------------------------------- */
        xmlWriter.writeEndElement();
        xmlWriter.writeEndDocument();

        fileNum++;
    }
}

void dataToXml::sltWriteXml3D(QVector<float> surface, QVector<float> boundary, QVector<uint> X, QVector<uint> Y, uint width, uint height)
{
    dir = dialog.getExistingDirectory(0, tr("Open save directory..."), "/home/pi/Tester_Data", dialog.ShowDirsOnly | dialog.DontUseNativeDialog);
    dataArray1 = surface;
    dataArray2 = boundary;
    widthArray = X;
    heightArray = Y;
    width3D = width;
    height3D = height;
    writeXML3D();
}

void dataToXml::sltDataIn(QStringList loc, QStringList tme, QStringList tst, QStringList aLoc)
{
	if (calls == 0)
	{
        dir = dialog.getExistingDirectory(0, tr("Open save directory..."), "/home/pi/Tester_Data", dialog.ShowDirsOnly | dialog.DontUseNativeDialog);
		calls = 1;
	}
	location = loc;
	time = tme;
	testNum = tst;
	actualLocation = aLoc;
	doCalc();
    writeXMLResponseTime();
}

/**
 * @brief dataToXml::doCalc performs the responsetime calculation.
 *  Takes into account for the variance in acceleration, speed, and distances that happen on diffent tests.
 * @todo add checking method to validate data.
 */
void dataToXml::doCalc()
{
	responseTime.clear();
	float accel = 1.0 * 9810;		// G's to mm/s^2
	float speed = 145;				// mm/second
	int num = 0;
	for (int test = 0; test < actualLocation.length(); test++)
	{
		float handleLocation = actualLocation.at(test).toFloat();
		float distance;				
		float accelTime;
		float moveDist;
	
		accelTime = (speed / accel);
		distance = (0.5*(accel * qPow(accelTime, 2)));
		moveDist = (handleLocation / 100) - distance;
		totMoveTime = (((moveDist / speed) * 1000) + (accelTime) * 1000);
		qDebug() << accelTime << distance << handleLocation / 100 << totMoveTime;
		
		for (int i = 0; i < time.length(); i++)
		{	
			if (testNum.at(i).toInt() == num)
			{
				if (time.at(i).toFloat() == 0.0)
					responseTime << "-";
				else
					responseTime << QString::number(time.at(i).toFloat() - totMoveTime);
			}	
		}
		num++;
	}
	qDebug() << responseTime;
}
