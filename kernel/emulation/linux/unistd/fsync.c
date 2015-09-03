#include "fsync.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_fsync(int fd)
{
	return sys_fsync_nocancel(fd);
}

long sys_fsync_nocancel(int fd)
{
	int ret;

	ret = LINUX_SYSCALL1(__NR_fsync, fd);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

