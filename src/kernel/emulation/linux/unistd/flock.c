#include "flock.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_flock(int fd, int op)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_flock, fd, op);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
