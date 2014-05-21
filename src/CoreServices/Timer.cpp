#include "Timer.h"
#include <time.h>

void Microseconds(UnsignedWide* tickCount)
{
	struct timespec ts;

	if(clock_gettime(CLOCK_MONOTONIC,&ts) != 0)
		*tickCount = 0;
	else
	{
		*tickCount = ts.tv_sec * 1000000ll;
		*tickCount += ts.tv_nsec / 1000;
	}
}

