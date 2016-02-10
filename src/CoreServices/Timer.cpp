#include "Timer.h"
#include <mach/mach_time.h>
#include <stdint.h>

void Microseconds(UnsignedWide* tickCount)
{
	uint64_t time;

	time = mach_absolute_time();

	*tickCount = time / 1000000000ll;
	time -= *tickCount * 1000000000ll;

	*tickCount += time / 1000;
}

