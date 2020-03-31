#include "pthread_fchdir.h"
#include "../base.h"
#include "../errno.h"
#include <stddef.h>
#include "../mach/lkm.h"
#include "../../../../external/lkm/api.h"
#include "../fcntl/open.h"
#include "../fcntl/fcntl.h"
#include "../unistd/close.h"
#include "../unistd/dup.h"
#include "per_thread_wd.h"

long sys_pthread_fchdir(int newfd)
{
	int fd = get_perthread_wd();

	if (fd != LINUX_AT_FDCWD)
		close_internal(fd);

	if (newfd == -1)
		newfd = LINUX_AT_FDCWD; // return to per-process wd
	else
	{
		newfd = sys_dup(newfd);
		if (newfd < 0)
			return newfd;
		sys_fcntl(newfd, F_SETFD, FD_CLOEXEC);
	}

	set_perthread_wd(newfd);
	return 0;
}

