#ifndef LINUX_SETITIMER_H
#define LINUX_SETITIMER_H
#include "gettimeofday.h"

struct bsd_itimerval
{
	struct bsd_timeval it_interval;
	struct bsd_timeval it_value;
};

struct linux_itimerval
{
	struct linux_timeval it_interval;
	struct linux_timeval it_value;
};

long sys_setitimer(int which, struct bsd_itimerval* itv,
		struct bsd_itimerval* oitv);

#endif

