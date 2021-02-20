#include "symlinkat.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <sys/errno.h>
#include "../common_at.h"
#include "../vchroot_expand.h"
#include <lkm/api.h>
#include <mach/lkm.h>
#include "../bsdthread/per_thread_wd.h"

long sys_symlinkat(const char* path, int fd, const char* link)
{
	int ret;
	struct vchroot_expand_args vc;

	vc.flags = 0;
	vc.dfd = get_perthread_wd();
	
	strcpy(vc.path, link);

	ret = vchroot_expand(&vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	ret = LINUX_SYSCALL(__NR_symlinkat, path, vc.dfd, vc.path);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
