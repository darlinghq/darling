#include "./sys/timerfd.h"
#include "../errno.h"
#include "../base.h"
#include <asm/unistd.h>

extern long cerror(int __err);

int timerfd_create (int __clock_id, int __flags)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_timerfd_create, __clock_id, __flags);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(rv));
		return -1;
	}

	return rv;
}

