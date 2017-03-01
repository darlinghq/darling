#include "./sys/epoll.h"
#include "../errno.h"
#include "../base.h"
#include <linux-syscalls/linux.h>

extern long cerror(int __err);

VISIBLE
int epoll_ctl (int __epfd, int __op, int __fd,
		struct epoll_event *__event)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_epoll_ctl, __epfd, __op, __fd,
			__event);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(-rv));
		return -1;
	}

	return rv;
}

