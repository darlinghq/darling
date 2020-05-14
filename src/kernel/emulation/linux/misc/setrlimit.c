#include "setrlimit.h"
#include "../base.h"
#include "../errno.h"
#include <sys/errno.h>
#include <linux-syscalls/linux.h>

long sys_setrlimit(unsigned int which, const struct rlimit* rlp)
{
	int ret;
	struct rlimit lim = { rlp->rlim_cur, rlp->rlim_max };

	which = rlimit_bsd_to_linux(which);
	if (which == -1)
		return -EINVAL;

	if (which == LINUX_RLIMIT_NOFILE)
	{
		lim.rlim_cur++;
		lim.rlim_max++;
	}

	ret = LINUX_SYSCALL(__NR_prlimit64, 0, which, &lim, 0);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

