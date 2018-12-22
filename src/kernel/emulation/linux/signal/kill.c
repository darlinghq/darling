#include "kill.h"
#include "../base.h"
#include "../errno.h"
#include "../../../../../platform-include/sys/errno.h"
#include "duct_signals.h"
#include <linux-syscalls/linux.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"
#include "sigexc.h"

extern int getpid(void);

long sys_kill(int pid, int signum, int posix)
{
	int ret, linux_signum;
	char buf[128];

	__simple_sprintf(buf, "sys_kill pid=%d bsd_signum=%d\n", pid, signum);
	lkm_call(0x1028, buf);

	// If we're stopping a process we're debugging, do an emulated SIGSTOP
	// so that the tracee has a chance to talk to us before stopping.
	if (signum == SIGSTOP && pid > 0)
	{
		int tracer = lkm_call(NR_get_tracer, (void*)(long)pid);
		if (tracer == getpid())
		{
			linux_sigqueue(pid, SIGNAL_SIGEXC_THUPDATE, -LINUX_SIGSTOP);
			return 0;
		}
	}

	linux_signum = signum_bsd_to_linux(signum);
	if (!linux_signum)
		return -EINVAL;

	__simple_sprintf(buf, "sys_kill pid=%d linux_signum=%d\n", pid, linux_signum);
	lkm_call(0x1028, buf);
	ret = LINUX_SYSCALL(__NR_kill, pid, linux_signum);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

