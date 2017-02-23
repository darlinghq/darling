#include "msync.h"
#include "duct_mman.h"
#include "../errno.h"
#include "../base.h"
#include <linux-syscalls/linux.h>

int msync_flags_bsd_to_linux(int flags)
{
	int oflags = 0;

	if (flags & BSD_MS_ASYNC)
		oflags |= LINUX_MS_ASYNC;
	if (flags & BSD_MS_SYNC)
		oflags |= LINUX_MS_SYNC;
	if (flags & BSD_MS_INVALIDATE)
		oflags |= LINUX_MS_INVALIDATE;

	return oflags;
}

long sys_msync(void* addr, unsigned long len, int flags)
{
	return sys_msync_nocancel(addr, len, flags);
}

long sys_msync_nocancel(void* addr, unsigned long len, int flags)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_msync, addr, len, msync_flags_bsd_to_linux(flags));
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
