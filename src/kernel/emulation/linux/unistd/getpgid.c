#include "getpgid.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_getpgid(int pid)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_getpgid, pid);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

