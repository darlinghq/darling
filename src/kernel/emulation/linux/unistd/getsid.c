#include "getsid.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_getsid(int pid)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_getsid, pid);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

