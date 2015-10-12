#include "./sys/epoll.h"
#include "../errno.h"
#include "../base.h"
#include <asm/unistd.h>

extern long cerror(int __err);

int epoll_ctl (int __epfd, int __op, int __fd,
		struct epoll_event *__event)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_epoll_ctl, __epfd, __op, __fd,
			__event);
	if (rv < 0)
	{
		cerror(rv);
		return -1;
	}

	return rv;
}

