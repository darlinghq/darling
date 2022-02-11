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

extern int driver_fd;
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

		// close the old RPC fd
		// FIXME: we actually have to close ALL the old RPC fds for any threads the parent process may have had
		close_internal((int)(intptr_t)_os_tsd_get_direct(__TSD_DSERVER_RPC_FD));

		// create a new dserver RPC socket
		int new_rpc_fd = __dserver_new_socket();
		if (new_rpc_fd < 0) {
			// we can't do anything if we don't get our own separate connection to darlingserver
			__simple_printf("Failed to create socket after fork\n");
			__simple_abort();
		}

		// set the new RPC fd
		_os_tsd_set_direct(__TSD_DSERVER_RPC_FD, (void*)(intptr_t)new_rpc_fd);
		driver_fd = new_rpc_fd;

		// update the fd stored in dyld, too
		void (*dyld_set_mach_driver_fd)(int fd);
		_libkernel_functions->dyld_func_lookup("__dyld_set_mach_driver_fd", (void**)&dyld_set_mach_driver_fd);
		dyld_set_mach_driver_fd(new_rpc_fd);

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

