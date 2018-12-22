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
	char buf[4096];
	struct vchroot_expand_args vc;

	vc.flags = VCHROOT_FOLLOW;
	vc.dfd = get_perthread_wd();
	
	strcpy(vc.path, path);

	rv = lkm_call(NR_vchroot_expand, &vc);
	if (rv < 0)
		return errno_linux_to_bsd(rv);

	newfd = sys_open(vc.path, BSD_O_RDONLY | BSD_O_DIRECTORY | BSD_O_CLOEXEC, 0);
	if (newfd < 0)
		return newfd;

	if (vc.dfd != LINUX_AT_FDCWD)
		sys_close(vc.dfd);

	set_perthread_wd(newfd);
	return 0;
}

