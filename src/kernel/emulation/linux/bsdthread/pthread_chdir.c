#include "pthread_chdir.h"
#include "../base.h"
#include "../errno.h"
#include <stddef.h>
#include "../mach/lkm.h"
#include "../../../../external/lkm/api.h"
#include "../fcntl/open.h"
#include "../unistd/close.h"
#include "per_thread_wd.h"

long sys_pthread_chdir(const char* path)
{
	int rv, newfd;

	newfd = sys_open(path, BSD_O_RDONLY | BSD_O_DIRECTORY | BSD_O_CLOEXEC, 0);
	if (newfd < 0)
		return newfd;

	set_perthread_wd(newfd);
	return 0;
}

