#ifndef LINUX_GETTIMEOFDAY_H
#define LINUX_GETTIMEOFDAY_H
#include <stdint.h>

struct bsd_timeval
{
	unsigned long tv_sec;
	int32_t tv_usec;
};

struct linux_timeval
{
	unsigned long tv_sec;
	long tv_usec;
};

struct timezone;

long sys_gettimeofday(struct bsd_timeval* tv, struct timezone* tz);

#endif

