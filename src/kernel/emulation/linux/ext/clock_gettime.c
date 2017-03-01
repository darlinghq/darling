#include "./sys/linux_time.h"
#include "../errno.h"
#include "../base.h"
#include <linux-syscalls/linux.h>

extern long cerror(int __err);

VISIBLE
int clock_gettime (int __clockid, struct timespec* __res)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_clock_gettime, __clockid, __res);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(-rv));
		return -1;
	}

	return rv;
}

