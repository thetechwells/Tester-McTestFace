/** \file timeManager.h
*	Author: Trenton Wells
*	Created: 8/26/2016
*/

#pragma once

#include <time.h>

class timeManager
{
public:
	timeManager();
	~timeManager();
	
	void startTime();
	void startTimeWhile();
	double endTime();
	double endTimeWhile();
	void waitMs(unsigned int);
	
private:
	timespec start;
	timespec startWait;
	timespec startWhile;
	timespec end;
	timespec endWait;
	timespec endWhile;
	timespec diff;
	timespec diff2;
	timespec diff3;
	
};

