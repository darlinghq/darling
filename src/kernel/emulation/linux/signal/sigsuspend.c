#include "sigsuspend.h"
#include "../base.h"
#include "../errno.h"
#include "duct_signals.h"
#include <stddef.h>
#include <asm/unistd.h>

long sys_sigsuspend(sigset_t set)
{
	return sys_sigsuspend_nocancel(set);
}

long sys_sigsuspend_nocancel(sigset_t set)
{
	linux_sigset_t set_linux;
	int ret;

	sigset_bsd_to_linux(&set, &set_linux);

	ret = LINUX_SYSCALL(__NR_rt_sigsuspend, &set_linux, 8);

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

