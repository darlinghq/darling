#include "fstatat.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include "../common_at.h"
#include "../vchroot_expand.h"
#include <lkm/api.h>
#include <mach/lkm.h>
#include <linux-syscalls/linux.h>
#include "../../../../../platform-include/sys/errno.h"

extern char* strcpy(char* dst, const char* src);

#define AT_SYMLINK_NOFOLLOW 0x20

long sys_fstatat(int fd, const char* path, struct stat* stat, int flag)
{
	int ret;
	struct linux_stat lstat;
	int linux_flags;

	if (!path)
		return -EFAULT;

	struct vchroot_expand_args vc;
	vc.flags = (flag & AT_SYMLINK_NOFOLLOW) ? 0 : VCHROOT_FOLLOW;
	vc.dfd = atfd(fd);
	
	strcpy(vc.path, path);
	ret = vchroot_expand(&vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	linux_flags = atflags_bsd_to_linux(flag);

#ifdef __NR_newfstatat
	ret = LINUX_SYSCALL(__NR_newfstatat, vc.dfd, vc.path, &lstat, linux_flags);
#else
	ret = LINUX_SYSCALL(__NR_fstatat64, vc.dfd, vc.path, &lstat, linux_flags);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	stat_linux_to_bsd(&lstat, stat);

	return 0;
}

long sys_fstatat64(int fd, const char* path, struct stat64* stat, int flag)
{
	int ret;
	struct linux_stat lstat;
	int linux_flags;

	if (!path)
		return -EFAULT;

	struct vchroot_expand_args vc;
	vc.flags = (flag & AT_SYMLINK_NOFOLLOW) ? 0 : VCHROOT_FOLLOW;
	vc.dfd = atfd(fd);
	
	strcpy(vc.path, path);
	ret = vchroot_expand(&vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	linux_flags = atflags_bsd_to_linux(flag);

#ifdef __NR_newfstatat
	ret = LINUX_SYSCALL(__NR_newfstatat, vc.dfd, vc.path, &lstat, linux_flags);
#else
	ret = LINUX_SYSCALL(__NR_fstatat64, vc.dfd, vc.path, &lstat, linux_flags);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	stat_linux_to_bsd64(&lstat, stat);

	return 0;
}

