#include "lstat.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <lkm/api.h>
#include <mach/lkm.h>
#include "../bsdthread/per_thread_wd.h"

long sys_lstat(const char* path, struct stat* stat)
{
	int ret;
	struct linux_stat lstat;
	struct vchroot_expand_args vc;

	vc.flags = 0;
	vc.dfd = get_perthread_wd();
	
	strcpy(vc.path, path);

	ret = lkm_call(NR_vchroot_expand, &vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

#ifdef __NR_lstat64
	ret = LINUX_SYSCALL(__NR_lstat64, vc.path, &lstat);
#else
	ret = LINUX_SYSCALL(__NR_lstat, vc.path, &lstat);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	stat_linux_to_bsd(&lstat, stat);

	return 0;
}

long sys_lstat64(const char* path, struct stat64* stat)
{
	int ret;
	struct linux_stat lstat;
	struct vchroot_expand_args vc;

	vc.flags = 0;
	vc.dfd = get_perthread_wd();
	
	strcpy(vc.path, path);

	ret = lkm_call(NR_vchroot_expand, &vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

#ifdef __NR_lstat64
	ret = LINUX_SYSCALL(__NR_lstat64, vc.path, &lstat);
#else
	ret = LINUX_SYSCALL(__NR_lstat, vc.path, &lstat);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	stat_linux_to_bsd64(&lstat, stat);

	return 0;
}
