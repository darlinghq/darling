#include "pipe.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_pipe(int fd[2])
{
	int ret;

	ret = LINUX_SYSCALL(__NR_pipe, fd);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

