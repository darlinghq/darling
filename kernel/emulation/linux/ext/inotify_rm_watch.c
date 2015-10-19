#include "./sys/inotify.h"
#include "../errno.h"
#include "../base.h"
#include <asm/unistd.h>

extern long cerror(int __err);

int inotify_rm_watch (int __fd, int __wd)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_inotify_rm_watch, __fd, __wd);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(rv));
		return -1;
	}

	return rv;
}

