#include "fstat.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_fstat(int fd, struct stat* stat)
{
	int ret;
	struct linux_stat lstat;

#ifdef __NR_fstat64
	ret = LINUX_SYSCALL(__NR_fstat64, fd, &lstat);
#else
	ret = LINUX_SYSCALL(__NR_fstat, fd, &lstat);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	stat_linux_to_bsd(&lstat, stat);

	return 0;
}

long sys_fstat64(int fd, struct stat64* stat)
{
	int ret;
	struct linux_stat lstat;

#ifdef __NR_fstat64
	ret = LINUX_SYSCALL(__NR_fstat64, fd, &lstat);
#else
	ret = LINUX_SYSCALL(__NR_fstat, fd, &lstat);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	stat_linux_to_bsd64(&lstat, stat);

	return 0;
}

