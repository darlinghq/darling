#include "fremovexattr.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_fremovexattr(int fd, const char* name, int options)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_fremovexattr, fd, name);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return ret;
}

