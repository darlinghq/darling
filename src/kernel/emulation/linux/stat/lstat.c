#include "lstat.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_lstat(const char* path, struct stat* stat)
{
	int ret;
	struct linux_stat lstat;

#ifdef __NR_lstat64
	ret = LINUX_SYSCALL(__NR_lstat64, path, &lstat);
#else
	ret = LINUX_SYSCALL(__NR_lstat, path, &lstat);
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

#ifdef __NR_lstat64
	ret = LINUX_SYSCALL(__NR_lstat64, path, &lstat);
#else
	ret = LINUX_SYSCALL(__NR_lstat, path, &lstat);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	stat_linux_to_bsd64(&lstat, stat);

	return 0;
}
