#include "pwrite.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_pwrite(int fd, const void* mem, int len, long long ofs)
{
	return sys_pwrite_nocancel(fd, mem, len, ofs);
}

long sys_pwrite_nocancel(int fd, const void* mem, int len, long long ofs)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_pwrite64, fd, mem, len, LL_ARG(ofs));
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return ret;
}

