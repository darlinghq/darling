#include "fork.h"
#include "../base.h"
#include "../errno.h"
#include "../signal/duct_signals.h"
#include "../../../mach_server/client/lkm.h"
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
	//else if (ret == 0)
	//	mach_driver_init(); // re-open /dev/mach

	return ret;
}

