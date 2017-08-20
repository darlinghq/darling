#include "read.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../bsdthread/cancelable.h"

long sys_read(int fd, void* mem, int len)
{
	CANCELATION_POINT();
	return sys_read_nocancel(fd, mem, len);
}

long sys_read_nocancel(int fd, void* mem, int len)
{
	int ret;

	ret = LINUX_SYSCALL3(__NR_read, fd, mem, len);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return ret;
}

