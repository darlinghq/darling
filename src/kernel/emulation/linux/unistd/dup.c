#include "dup.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_dup(int fd)
{
	int ret;

	ret = LINUX_SYSCALL1(__NR_dup, fd);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

