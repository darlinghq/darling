#include "read.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

int sys_read(int fd, void* mem, int len)
{
	return sys_read_nocancel(fd, mem, len);
}

int sys_read_nocancel(int fd, void* mem, int len)
{
	int ret;

	ret = LINUX_SYSCALL3(__NR_read, fd, mem, len);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return ret;
}

