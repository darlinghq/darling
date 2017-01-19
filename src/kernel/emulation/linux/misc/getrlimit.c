#include "getrlimit.h"
#include "../base.h"
#include "../errno.h"
#include "../../../../../platform-include/sys/errno.h"
#include <linux-syscalls/linux.h>

long sys_getrlimit(unsigned int which, struct rlimit* rlp)
{
	int ret;

	which = rlimit_bsd_to_linux(which);
	if (which == -1)
		return -EINVAL;

	ret = LINUX_SYSCALL(__NR_prlimit64, 0, which, 0, rlp);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

unsigned int rlimit_bsd_to_linux(unsigned int which)
{
	which &= ~BSD_RLIMIT_POSIX;

	switch (which)
	{
		case BSD_RLIMIT_CPU:
			return LINUX_RLIMIT_CPU;
		case BSD_RLIMIT_FSIZE:
			return LINUX_RLIMIT_FSIZE;
		case BSD_RLIMIT_DATA:
			return LINUX_RLIMIT_DATA;
		case BSD_RLIMIT_STACK:
			return LINUX_RLIMIT_STACK;
		case BSD_RLIMIT_CORE:
			return LINUX_RLIMIT_CORE;
		case BSD_RLIMIT_AS:
			return LINUX_RLIMIT_AS;
		case BSD_RLIMIT_MEMLOCK:
			return LINUX_RLIMIT_MEMLOCK;
		case BSD_RLIMIT_NPROC:
			return LINUX_RLIMIT_NPROC;
		case BSD_RLIMIT_NOFILE:
			return LINUX_RLIMIT_NOFILE;
		default:
			return -1;
	}
}

