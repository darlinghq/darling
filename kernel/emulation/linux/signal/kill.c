#include "kill.h"
#include "../base.h"
#include "../errno.h"
#include "../../../../platform-include/sys/errno.h"
#include "duct_signals.h"
#include <asm/unistd.h>

long sys_kill(int pid, int signum, int posix)
{
	int ret, linux_signum;

	linux_signum = signum_bsd_to_linux(signum);
	if (!linux_signum)
		return -EINVAL;

	ret = LINUX_SYSCALL(__NR_kill, pid, linux_signum);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

