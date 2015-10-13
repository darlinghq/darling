#include "./sys/timerfd.h"
#include "../errno.h"
#include "../base.h"
#include <asm/unistd.h>

extern long cerror(int __err);

int timerfd_settime (int __fd, int __flags,
		const struct itimerspec* __utmr,
		struct itimerspec* __otmr)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_timerfd_settime, __fd, __flags, __utmr, __otmr);
	if (rv < 0)
	{
		cerror(rv);
		return -1;
	}

	return rv;
}

