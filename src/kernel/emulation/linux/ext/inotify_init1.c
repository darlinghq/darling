#include "./sys/inotify.h"
#include "../errno.h"
#include "../base.h"
#include <asm/unistd.h>

extern long cerror(int __err);

int inotify_init1 (int __flags)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_inotify_init1, __flags);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(rv));
		return -1;
	}

	return rv;
}

