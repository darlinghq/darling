#include "Timer.h"
#include <mach/mach_time.h>
#include <stdint.h>

void Microseconds(UnsignedWide* tickCount)
{
	uint64_t time;

	time = mach_absolute_time();

	*reinterpret_cast<uint64_t*>(tickCount) = time / 1000000000ll;
	time -= *reinterpret_cast<uint64_t*>(tickCount) * 1000000000ll;

	*reinterpret_cast<uint64_t*>(tickCount) += time / 1000;
}

