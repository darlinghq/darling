#include "fstatat.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include "../common_at.h"
#include <linux-syscalls/linux.h>

long sys_fstatat(int fd, const char* path, struct stat* stat, int flag)
{
	int ret;
	struct linux_stat lstat;
	int linux_flags;

	linux_flags = atflags_bsd_to_linux(flag);

#ifdef __NR_newfstatat
	ret = LINUX_SYSCALL(__NR_newfstatat, atfd(fd), path, &lstat, linux_flags);
#else
	ret = LINUX_SYSCALL(__NR_fstatat64, atfd(fd), path, &lstat, linux_flags);
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

	linux_flags = atflags_bsd_to_linux(flag);

#ifdef __NR_newfstatat
	ret = LINUX_SYSCALL(__NR_newfstatat, atfd(fd), path, &lstat, linux_flags);
#else
	ret = LINUX_SYSCALL(__NR_fstatat64, atfd(fd), path, &lstat, linux_flags);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	stat_linux_to_bsd64(&lstat, stat);

	return 0;
}

