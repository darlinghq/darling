#include "poll.h"
#include "../base.h"
#include "../errno.h"
#include <stddef.h>
#include <linux-syscalls/linux.h>

long sys_poll(struct pollfd* fds, unsigned int nfds, int timeout)
{
	return sys_poll_nocancel(fds, nfds, timeout);
}

long sys_poll_nocancel(struct pollfd* fds, unsigned int nfds, int timeout)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_poll, fds, nfds, timeout);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

