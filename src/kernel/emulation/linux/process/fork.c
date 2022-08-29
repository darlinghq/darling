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
		int newReadFd = __dserver_process_lifetime_pipe_refresh();

		// guard it
		guard_entry_options_t options;
		options.close = __dserver_close_socket;
		guard_table_add(__dserver_per_thread_socket(), guard_flag_prevent_close | guard_flag_close_on_fork, &options);

		// only guard the lifetime pipe if it's used.
		if (newReadFd != -1)
		{
			options.close = __dserver_close_process_lifetime_pipe;
			guard_table_add(__dserver_get_process_lifetime_pipe(), guard_flag_prevent_close | guard_flag_close_on_fork, &options);
		}

		int dummy_stack_variable;
		if (dserver_rpc_checkin(true, &dummy_stack_variable, newReadFd) < 0) {
			// we can't do ANYTHING if darlingserver fails to acknowledge us
			__simple_printf("Failed to checkin with darlingserver after fork\n");
			__simple_abort();
		}
		if (wdfd >= 0)
			sys_fchdir(wdfd);

		__dserver_close_process_lifetime_pipe(newReadFd);
	}

	return ret;
}

