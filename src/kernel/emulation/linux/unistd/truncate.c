#include "truncate.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <lkm/api.h>
#include "../bsdthread/per_thread_wd.h"
#include "../vchroot_expand.h"
#include <mach/lkm.h>

long sys_truncate(const char* path, long long length)
{
	int ret;
	struct vchroot_expand_args vc;

	vc.flags = VCHROOT_FOLLOW;
	vc.dfd = get_perthread_wd();
	
	strcpy(vc.path, path);

	ret = vchroot_expand(&vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

#ifdef __NR_truncate64
	ret = LINUX_SYSCALL(__NR_truncate64, vc.path, LL_ARG(length));
#else
	ret = LINUX_SYSCALL(__NR_truncate, vc.path, LL_ARG(length));
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
