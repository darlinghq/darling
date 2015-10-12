#include "./sys/epoll.h"
#include "../errno.h"
#include "../base.h"
#include <asm/unistd.h>

extern long cerror(int __err);

int epoll_create (int __size)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_epoll_create, __size);
	if (rv < 0)
	{
		cerror(rv);
		return -1;
	}

	return rv;
}

