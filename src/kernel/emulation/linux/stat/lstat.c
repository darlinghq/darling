#include "lstat.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <lkm/api.h>
#include <mach/lkm.h>
#include "../vchroot_expand.h"
#include "../bsdthread/per_thread_wd.h"
#include "../../../../../platform-include/sys/errno.h"

long sys_lstat(const char* path, struct stat* stat)
{
	int ret;
	struct linux_stat lstat;
	struct vchroot_expand_args vc;

	if (!path)
		return -EFAULT;

	vc.flags = 0;
	vc.dfd = get_perthread_wd();
	
	strcpy(vc.path, path);

	ret = vchroot_expand(&vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	#ifdef defined(__NR_lstat64)
		ret = LINUX_SYSCALL(__NR_lstat64, vc.path, &lstat);
	#elif defined(__NR_lstat)
		ret = LINUX_SYSCALL(__NR_lstat, vc.path, &lstat);
	#else
		#if defined(__NR_newfstatat)
			int status = LINUX_SYSCALL(__NR_newfstatat, LINUX_AT_FDCWD, vc.path, &lstat, LINUX_AT_SYMLINK_NOFOLLOW);
		#else
			int status = LINUX_SYSCALL(__NR_fstatat64, LINUX_AT_FDCWD, vc.path, &lstat, LINUX_AT_SYMLINK_NOFOLLOW);
		#endif
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

	ret = vchroot_expand(&vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	#ifdef defined(__NR_lstat64)
		ret = LINUX_SYSCALL(__NR_lstat64, vc.path, &lstat);
	#elif defined(__NR_lstat)
		ret = LINUX_SYSCALL(__NR_lstat, vc.path, &lstat);
	#else
		#if defined(__NR_newfstatat)
			int status = LINUX_SYSCALL(__NR_newfstatat, LINUX_AT_FDCWD, vc.path, &lstat, LINUX_AT_SYMLINK_NOFOLLOW);
		#else
			int status = LINUX_SYSCALL(__NR_fstatat64, LINUX_AT_FDCWD, vc.path, &lstat, LINUX_AT_SYMLINK_NOFOLLOW);
		#endif
	#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	stat_linux_to_bsd64(&lstat, stat);

	return 0;
}
