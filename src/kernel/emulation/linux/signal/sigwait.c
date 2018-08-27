#include "sigwait.h"
#include "../base.h"
#include "../errno.h"
#include "duct_signals.h"
#include <stddef.h>
#include <linux-syscalls/linux.h>
#include "../bsdthread/cancelable.h"
#include "sigaction.h"

long sys_sigwait(sigset_t* set, int* sig)
{
	CANCELATION_POINT();
	return sys_sigwait_nocancel(set, sig);
}

long sys_sigwait_nocancel(sigset_t* set, int* sig)
{
	linux_sigset_t set_linux;
	int ret;
	struct linux_siginfo si;

	sigset_bsd_to_linux(set, &set_linux);

	ret = LINUX_SYSCALL(__NR_rt_sigtimedwait, &set_linux, &si, NULL, 8);

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);
	else if (sig)
		*sig = signum_linux_to_bsd(si.si_signo);

	return ret;
}

