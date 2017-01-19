#include "./sys/epoll.h"
#include "../errno.h"
#include "../base.h"
#include <linux-syscalls/linux.h>

extern long cerror(int __err);

VISIBLE
int epoll_create (int __size)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_epoll_create, __size);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(rv));
		return -1;
	}

	return rv;
}

