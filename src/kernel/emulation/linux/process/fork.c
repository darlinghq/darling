#include "fork.h"
#include "../base.h"
#include "../errno.h"
#include "../signal/duct_signals.h"
#include <linux-syscalls/linux.h>

long sys_fork(void)
{
	int ret;

#ifdef SYS_fork
	ret = LINUX_SYSCALL(__NR_fork);
#else
	ret = LINUX_SYSCALL(__NR_clone, LINUX_SIGCHLD, 0);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

