#include "readv.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_readv(int fd, struct iovec* iovp, unsigned int len)
{
	return sys_readv_nocancel(fd, iovp, len);
}

long sys_readv_nocancel(int fd, struct iovec* iovp, unsigned int len)
{
	int ret;

	ret = LINUX_SYSCALL3(__NR_readv, fd, iovp, len);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return ret;
}

