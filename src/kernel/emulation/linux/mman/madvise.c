#include "madvise.h"
#include "../errno.h"
#include "../base.h"
#include <sys/errno.h>
#include <linux-syscalls/linux.h>

#define MADV_FREE 5
#define MADV_FREE_REUSABLE 7
#define MADV_FREE_REUSE 8

long sys_madvise(void* addr, unsigned long len, int advice)
{
	int ret;

	if (advice == MADV_FREE_REUSABLE || advice == MADV_FREE_REUSE)
		advice = MADV_FREE;

	// advice < 0 are identical between OS X and Linux
	// advice >= 5 are specific to OS X/BSD
	if (advice >= 5)
		return 0;

	ret = LINUX_SYSCALL(__NR_madvise, addr, len, advice);

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

