#include "./sys/inotify.h"
#include "../errno.h"
#include "../base.h"
#include <asm/unistd.h>

extern long cerror(int __err);

int inotify_init (void)
{
	int rv;

	rv = LINUX_SYSCALL(__NR_inotify_init);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(rv));
		return -1;
	}

	return rv;
}

