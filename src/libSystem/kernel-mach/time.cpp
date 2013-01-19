#include "config.h"
#include "time.h"
#include <ctime>
#include <mach/kern_return.h>

uint64_t mach_absolute_time()
{
	timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == -1)
		return 0;
	else
		return ts.tv_nsec * 1000000000ll + ts.tv_nsec;
}

int mach_timebase_info(struct mach_timebase_info* info)
{
	info->numer = 1;
	info->denom = 1; // we provide nanoseconds
	return KERN_SUCCESS;
}
