#include "./sys/timerfd.h"
#include "../errno.h"
#include "../base.h"
#include <asm/unistd.h>

extern long cerror(int __err);

int timerfd_gettime (int __fd, struct itimerspec* __otmr)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_timerfd_gettime, __fd, __otmr);
	if (rv < 0)
	{
		cerror(rv);
		return -1;
	}

	return rv;
}

