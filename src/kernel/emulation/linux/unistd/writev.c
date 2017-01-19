#include "writev.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_writev(int fd, struct iovec* iovp, unsigned int len)
{
	return sys_writev_nocancel(fd, iovp, len);
}

long sys_writev_nocancel(int fd, struct iovec* iovp, unsigned int len)
{
	int ret;

	ret = LINUX_SYSCALL3(__NR_writev, fd, iovp, len);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return ret;
}

