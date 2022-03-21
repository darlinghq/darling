#include "fork.h"
#include "../base.h"
#include "../errno.h"
#include "../signal/duct_signals.h"
#include "../bsdthread/per_thread_wd.h"
#include "../unistd/fchdir.h"
#include <linux-syscalls/linux.h>

#include <darlingserver/rpc.h>
#include "../simple.h"
#include "../elfcalls_wrapper.h"
#include "../mach/lkm.h"
#include "../unistd/close.h"
#include "../../../libsyscall/wrappers/_libkernel_init.h"
#include "../guarded/table.h"

extern _libkernel_functions_t _libkernel_functions;

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

		// the old RPC FD will be closed in `guard_table_postfork_child`;
		// we don't need to close it ourselves.
		// that should also take care of closing descriptors for any other threads.
		guard_table_postfork_child();

		// create a new dserver RPC socket
		__dserver_per_thread_socket_refresh();

		// guard it
		guard_table_add(__dserver_per_thread_socket(), guard_flag_prevent_close | guard_flag_close_on_fork);

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

