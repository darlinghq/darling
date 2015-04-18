#include <mach/mach_time.h>
#include <mach/kern_return.h>

kern_return_t		mach_timebase_info(
						mach_timebase_info_t	info)
{
	return KERN_FAILURE;
}

kern_return_t		mach_wait_until(
						uint64_t		deadline)
{
	return KERN_FAILURE;
}


uint64_t mach_absolute_time(void)
{
	return 0;
}
