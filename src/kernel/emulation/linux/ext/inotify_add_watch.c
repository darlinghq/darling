#include "./sys/inotify.h"
#include "../errno.h"
#include "../base.h"
#include <asm/unistd.h>

extern long cerror(int __err);

VISIBLE
int inotify_add_watch (int __fd, const char *__name, uint32_t __mask)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_inotify_add_watch, __fd, __name, __mask);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(rv));
		return -1;
	}

	return rv;
}

