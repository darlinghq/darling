#include <mach/mach_time.h>
#include <mach/kern_return.h>
#ifndef _MACH_TIME_H
#define _MACH_TIME_H

kern_return_t		mach_timebase_info_trap_impl(
						mach_timebase_info_t	info);
kern_return_t		mach_wait_until_impl(
						uint64_t		deadline);

#endif

