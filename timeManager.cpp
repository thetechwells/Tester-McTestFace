#include "timeManager.h"

timeManager::timeManager()
{
	
}

timeManager::~timeManager()
{
	
}

void timeManager::startTime()
{
	clock_gettime(CLOCK_REALTIME, &start);
}

double timeManager::endTime()
{
	clock_gettime(CLOCK_REALTIME, &end);
	
	if ((end.tv_nsec - start.tv_nsec) < 0) {
		diff.tv_sec = end.tv_sec - start.tv_sec - 1;
		diff.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	}
	else {
		diff.tv_sec = end.tv_sec - start.tv_sec;
		diff.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
			
	// add the usec to the sec and convert to msec
	return (diff.tv_sec + (diff.tv_nsec * 0.000000001f)) * 1000;
}

void timeManager::waitMs(unsigned int time)
{
	clock_gettime(CLOCK_REALTIME, &startWait);
	double waiting = 0;
	
	while (waiting <= time)
	{
		clock_gettime(CLOCK_REALTIME, &endWait);
	
		if ((endWait.tv_nsec - startWait.tv_nsec) < 0) {
			diff2.tv_sec = endWait.tv_sec - startWait.tv_sec - 1;
			diff2.tv_nsec = 1000000000 + endWait.tv_nsec - startWait.tv_nsec;
		}
		else {
			diff2.tv_sec = endWait.tv_sec - startWait.tv_sec;
			diff2.tv_nsec = endWait.tv_nsec - startWait.tv_nsec;
		}
		// add the usec to the sec and convert to msec
		waiting = (diff2.tv_sec + (diff2.tv_nsec * 0.000000001f)) * 1000;
	}
}

void timeManager::startTimeWhile()
{
	clock_gettime(CLOCK_REALTIME, &startWhile);
}

double timeManager::endTimeWhile()
{
	clock_gettime(CLOCK_REALTIME, &endWhile);
	
	if ((endWhile.tv_nsec - start.tv_nsec) < 0) {
		diff3.tv_sec = endWhile.tv_sec - startWhile.tv_sec - 1;
		diff3.tv_nsec = 1000000000 + endWhile.tv_nsec - startWhile.tv_nsec;
	}
	else {
		diff3.tv_sec = endWhile.tv_sec - startWhile.tv_sec;
		diff3.tv_nsec = endWhile.tv_nsec - startWhile.tv_nsec;
	}
			
	// add the usec to the sec and convert to msec
	return (diff3.tv_sec + (diff3.tv_nsec * 0.000000001f)) * 1000;
}