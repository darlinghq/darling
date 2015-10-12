#include "./sys/epoll.h"
#include "../errno.h"
#include "../base.h"
#include <asm/unistd.h>

extern long cerror(int __err);

int epoll_wait (int __epfd, struct epoll_event *__events,
		int __maxevents, int __timeout)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_epoll_wait, __epfd, __events, __maxevents,
			__timeout);
	if (rv < 0)
	{
		cerror(rv);
		return -1;
	}

	return rv;
}

