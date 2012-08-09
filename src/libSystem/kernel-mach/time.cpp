#include "config.h"
#include "time.h"
#include <ctime>
#include <mach/kern_return.h>

uint64_t mach_absolute_time()
{
	return clock();
}

int mach_timebase_info(struct mach_timebase_info* info)
{
	info->numer = 1;
	info->denom = CLOCKS_PER_SEC;
	return KERN_SUCCESS;
}
