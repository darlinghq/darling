#include "fork.h"
#include "../base.h"
#include "../errno.h"
#include "../signal/duct_signals.h"
#include "../bsdthread/per_thread_wd.h"
#include "../unistd/fchdir.h"
#include <linux-syscalls/linux.h>

#include <darlingserver/rpc.h>
#include "../simple.h"

long sys_fork(void)
{
	int ret;
	int wdfd = get_perthread_wd();

#ifdef SYS_fork
	ret = LINUX_SYSCALL(__NR_fork);
#else
	ret = LINUX_SYSCALL(__NR_clone, LINUX_SIGCHLD, 0);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);
	else if (ret == 0)
	{
		// in the child
		if (dserver_rpc_checkin(true) < 0) {
			// we can't do ANYTHING if darlingserver fails to acknowledge us
			__simple_printf("Failed to checkin with darlingserver after fork\n");
			__simple_abort();
		}
		if (wdfd >= 0)
			sys_fchdir(wdfd);
	}

	return ret;
}

