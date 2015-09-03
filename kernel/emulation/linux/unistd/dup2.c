#include "dup2.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_dup2(int fd_from, int fd_to)
{
	int ret;

	ret = LINUX_SYSCALL2(__NR_dup2, fd_from, fd_to);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

