#include "madvise.h"
#include "../errno.h"
#include "../base.h"
#include "../../../../../platform-include/sys/errno.h"
#include <linux-syscalls/linux.h>

long sys_madvise(void* addr, unsigned long len, int advice)
{
	int ret;

	// advice < 0 are identical between OS X and Linux
	// advice >= 5 are specific to OS X/BSD
	if (advice >= 5)
		return -ENOTSUP;

	ret = LINUX_SYSCALL(__NR_madvise, addr, len, advice);

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

