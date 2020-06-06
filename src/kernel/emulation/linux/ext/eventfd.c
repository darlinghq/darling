#include "./sys/eventfd.h"
#include "../errno.h"
#include "../base.h"
#include <linux-syscalls/linux.h>

extern long cerror(int __err);

VISIBLE
int __linux_eventfd (int __count, int __flags)
{
	int rv;

	#if defined(__x86_64__) || defined(__i386__)
		rv = LINUX_SYSCALL(__flags ? __NR_eventfd2 : __NR_eventfd, __count,
				__flags);
	#else
		// It's not clear if 0 is offically a valid flag argument.
		// But we don't really have a choice.
		rv = LINUX_SYSCALL(__NR_eventfd2, __count, __flags);
	#endif
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(-rv));
		return -1;
	}

	return rv;
}

