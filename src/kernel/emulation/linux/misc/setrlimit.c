#include "setrlimit.h"
#include "../base.h"
#include "../errno.h"
#include "../../../../../platform-include/sys/errno.h"
#include <linux-syscalls/linux.h>

long sys_setrlimit(unsigned int which, const struct rlimit* rlp)
{
	int ret;

	which = rlimit_bsd_to_linux(which);
	if (which == -1)
		return -EINVAL;

	ret = LINUX_SYSCALL(__NR_prlimit64, 0, which, rlp, 0);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

