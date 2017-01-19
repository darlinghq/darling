#include "close.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_close(int fd)
{
	return sys_close_nocancel(fd);
}

long sys_close_nocancel(int fd)
{
	int ret;

	ret = LINUX_SYSCALL1(__NR_close, fd);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

