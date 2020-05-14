#include "kill.h"
#include "../base.h"
#include "../errno.h"
#include <sys/errno.h>
#include "duct_signals.h"
#include <linux-syscalls/linux.h>
#include "../mach/lkm.h"
#include "../../../../external/lkm/api.h"
#include "sigexc.h"

extern int getpid(void);

long sys_kill(int pid, int signum, int posix)
{
	int ret, linux_signum;

	// If we're stopping a process we're debugging, do an emulated SIGSTOP
	// so that the tracee has a chance to talk to us before stopping.
	/*if (signum == SIGSTOP && pid > 0)
	{
		int tracer = lkm_call(NR_get_tracer, (void*)(long)pid);
		if (tracer == getpid())
		{
			linux_sigqueue(pid, SIGNAL_SIGEXC_THUPDATE, -LINUX_SIGSTOP);
			return 0;
		}
	}*/

	linux_signum = signum_bsd_to_linux(signum);
	if (!linux_signum)
		return -EINVAL;

	ret = LINUX_SYSCALL(__NR_kill, pid, linux_signum);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

