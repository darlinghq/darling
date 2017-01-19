#include "./sys/epoll.h"
#include "../errno.h"
#include "../base.h"
#include <linux-syscalls/linux.h>

extern long cerror(int __err);

VISIBLE
int epoll_wait (int __epfd, struct epoll_event *__events,
		int __maxevents, int __timeout)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_epoll_wait, __epfd, __events, __maxevents,
			__timeout);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(rv));
		return -1;
	}

	return rv;
}

