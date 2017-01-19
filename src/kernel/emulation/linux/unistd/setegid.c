#include "setegid.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_setegid(int egid)
{
	int ret;

	ret = LINUX_SYSCALL3(__NR_setresgid, -1, egid, -1);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

