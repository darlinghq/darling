#include "pthread_chdir.h"
#include "../base.h"
#include "../errno.h"
#include <stddef.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"
#include "../fcntl/open.h"
#include "../unistd/close.h"
#include "per_thread_wd.h"

long sys_pthread_chdir(const char* path)
{
	int rv, newfd;
	int fd = get_perthread_wd();
	char buf[4096];
	struct path_at_args args;

	args.fd = fd;
	args.path = path;
	args.path_out = buf;
	args.max_path_out = sizeof(buf);

	rv = lkm_call(NR_path_at, &args);
	if (rv)
		return errno_linux_to_bsd(rv);

	newfd = sys_open(buf, BSD_O_RDONLY | BSD_O_DIRECTORY | BSD_O_CLOEXEC, 0);
	if (newfd < 0)
		return newfd;

	if (fd != LINUX_AT_FDCWD)
		sys_close(fd);

	set_perthread_wd(newfd);
	return 0;
}

