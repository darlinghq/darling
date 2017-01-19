#include "./sys/linux_time.h"
#include "../errno.h"
#include "../base.h"
#include <linux-syscalls/linux.h>

extern long cerror(int __err);

VISIBLE
int clock_nanosleep (int __clockid, int __flags,
		const struct timespec* __req,
		struct timespec* __rem)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_clock_nanosleep, __clockid, __flags,
			__req, __rem);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(rv));
		return -1;
	}

	return rv;
}

