#ifndef EXT_TIMERFD_H
#define EXT_TIMERFD_H
#include <sys/types.h>

/*
 * The contents of this file will probably change or go away
 * when OS X implements this POSIX API.
 */

enum
{
	TFD_TIMER_ABSTIME = 1 << 0
};
enum
{
	TFD_CLOEXEC = 02000000,
	TFD_NONBLOCK = 00004000
};

#ifndef __THROW
#	define __THROW
#endif

#if !defined(__timespec_defined) && !defined(_STRUCT_TIMESPEC)
#	define __timespec_defined 1
#	define _STRUCT_TIMESPEC struct timespec
struct timespec
{
	long tv_sec;
	long tv_nsec;
};
#endif

struct itimerspec
{
	struct timespec it_interval;
	struct timespec it_value;
};


#define timerfd_create	__linux_timerfd_create
#define timerfd_settime	__linux_timerfd_settime
#define timerfd_gettime	__linux_timerfd_gettime

#ifndef CLOCK_MONOTONIC
#   define CLOCK_REALTIME               0
#   define CLOCK_MONOTONIC              1
#   define CLOCK_PROCESS_CPUTIME_ID     2
#   define CLOCK_THREAD_CPUTIME_ID      3
#   define CLOCK_MONOTONIC_RAW          4
#   define CLOCK_REALTIME_COARSE        5
#   define CLOCK_MONOTONIC_COARSE       6
#   define CLOCK_BOOTTIME               7
#   define CLOCK_REALTIME_ALARM         8
#   define CLOCK_BOOTTIME_ALARM         9
#   define CLOCK_TAI                    11
#endif

extern int timerfd_create (int __clock_id, int __flags) __THROW;

extern int timerfd_settime (int __ufd, int __flags,
							const struct itimerspec *__utmr,
							struct itimerspec *__otmr) __THROW;
extern int timerfd_gettime (int __ufd, struct itimerspec *__otmr) __THROW;


#endif

