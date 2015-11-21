#include "getrusage.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_getrusage(int who, void* rusage)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_getrusage, who, rusage);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

