#include "umask.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_umask(unsigned int newmask)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_umask, newmask);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

