#ifndef EXT_TIME_H
#define EXT_TIME_H
#include "timerfd.h"

#ifndef CLOCK_REALTIME
#	define CLOCK_REALTIME	0
#	define CLOCK_MONOTONIC	1
/* TODO: Other clocks */

#endif
#define TIMER_ABSTIME	1

#define clock_gettime		__linux_clock_gettime
#define clock_nanosleep		__linux_clock_nanosleep

extern int clock_gettime(int __clockid, struct timespec* __res);

extern int clock_nanosleep(int __clockid, int __flags,
		const struct timespec* __req, struct timespec* __rem);

#endif

