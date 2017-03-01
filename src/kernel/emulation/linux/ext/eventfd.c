#include "./sys/eventfd.h"
#include "../errno.h"
#include "../base.h"
#include <linux-syscalls/linux.h>

extern long cerror(int __err);

VISIBLE
int __linux_eventfd (int __count, int __flags)
{
	int rv;

	rv = LINUX_SYSCALL(__flags ? __NR_eventfd2 : __NR_eventfd, __count,
			__flags);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(-rv));
		return -1;
	}

	return rv;
}

