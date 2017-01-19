#include "waitid.h"
#include "../base.h"
#include "../errno.h"
#include "../signal/duct_signals.h"
#include "wait4.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>

long sys_waitid(int idtype, int id, struct simple_siginfo* si, int options)
{
	int ret, linux_options;

	linux_options = waitopts_bsd_to_linux(options);

	ret = LINUX_SYSCALL(__NR_waitid, idtype, id, si, linux_options);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	if (si != NULL)
	{
		si->si_signo = signum_linux_to_bsd(si->si_signo);
		si->si_errno = errno_linux_to_bsd(si->si_errno);
	}

	return ret;
}

