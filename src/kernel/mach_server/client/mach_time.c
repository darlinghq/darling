#include <mach/mach_time.h>
#include <mach/kern_return.h>
#include <errno.h>
#include <sys/linux_time.h>

#ifndef NSEC_PER_SEC
#	define NSEC_PER_SEC 1000000000ull
#endif

kern_return_t		mach_timebase_info_trap(
						mach_timebase_info_t	info)
{
	info->numer = NSEC_PER_SEC;
	info->denom = NSEC_PER_SEC;
	return KERN_SUCCESS;
}

kern_return_t		mach_wait_until(
						uint64_t		deadline)
{
	struct timespec ts = { deadline / NSEC_PER_SEC, deadline % NSEC_PER_SEC };
	
do_sleep:
	if (clock_sleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, 0) == -1)
	{
		if (errno == EINTR)
			goto do_sleep;
		return KERN_FAILURE;
	}
	
	return KERN_SUCCESS;
}


uint64_t mach_absolute_time(void)
{
	struct timespec ts;
	uint64_t out;
	
	clock_gettime(CLOCK_MONOTONIC, &ts);
	
	out = ts.tv_nsec;
	out += ts.tv_sec * NSEC_PER_SEC;
	
	return out;
}
