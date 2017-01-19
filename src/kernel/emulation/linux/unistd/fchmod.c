#include "fchmod.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_fchmod(int fd, int mode)
{
	int ret;

	ret = LINUX_SYSCALL2(__NR_fchmod, fd, mode);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

