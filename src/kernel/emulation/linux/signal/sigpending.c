#include "sigpending.h"
#include "../base.h"
#include "../errno.h"
#include "duct_signals.h"
#include <stddef.h>
#include <linux-syscalls/linux.h>

long sys_sigpending(sigset_t set)
{
	linux_sigset_t set_linux;
	int ret;

	sigset_bsd_to_linux(&set, &set_linux);

	ret = LINUX_SYSCALL(__NR_rt_sigpending, &set_linux, 8);

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

