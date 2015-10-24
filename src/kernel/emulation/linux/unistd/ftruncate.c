#include "ftruncate.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_ftruncate(int fd, long long length)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_ftruncate, fd, LL_ARG(length));
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

