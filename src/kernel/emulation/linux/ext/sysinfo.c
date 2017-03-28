#include "sysinfo.h"
#include "../errno.h"
#include "../base.h"
#include <linux-syscalls/linux.h>

int __linux_sysinfo(struct sysinfo* info)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_sysinfo, info);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(-rv));
		return -1;
	}

	return rv;
}

