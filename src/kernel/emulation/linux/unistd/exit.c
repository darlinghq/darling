#include "exit.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../elfcalls_wrapper.h"

long sys_exit(int status)
{
	int ret;

	native_exit(status);

	ret = LINUX_SYSCALL1(__NR_exit_group, status);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

