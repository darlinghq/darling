#include "utimes.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <lkm/api.h>
#include <mach/lkm.h>
#include "../vchroot_expand.h"
#include "../bsdthread/per_thread_wd.h"

long sys_utimes(const char* path, struct bsd_timeval* tv)
{
	int ret;
	struct linux_timeval ltv[2];

	if (tv) {
		ltv[0].tv_sec = tv[0].tv_sec;
		ltv[0].tv_usec = tv[0].tv_usec;
		ltv[1].tv_sec = tv[1].tv_sec;
		ltv[1].tv_usec = tv[1].tv_usec;
	}

	struct vchroot_expand_args vc;

	vc.flags = VCHROOT_FOLLOW;
	vc.dfd = get_perthread_wd();
	
	strcpy(vc.path, path);

	ret = vchroot_expand(&vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	#if defined(__NR_utimes)
		ret = LINUX_SYSCALL(__NR_utimes, vc.path, tv ? ltv: 0);
	#else
		ret = LINUX_SYSCALL(__NR_utimensat, LINUX_AT_FDCWD, vc.path, tv ? ltv: 0, 0);
	#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
