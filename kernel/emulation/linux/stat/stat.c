#include "stat.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_stat(const char* path, struct stat* stat)
{
	int ret;
	struct linux_stat lstat;

	// TODO: handle case conversion

#ifdef __NR_stat64
	ret = LINUX_SYSCALL(__NR_stat64, path, &lstat);
#else
	ret = LINUX_SYSCALL(__NR_stat, path, &lstat);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	stat_linux_to_bsd(&lstat, stat);

	return 0;
}

long sys_stat64(const char* path, struct stat64* stat)
{
	int ret;
	struct linux_stat lstat;

	// TODO: handle case conversion

#ifdef __NR_stat64
	ret = LINUX_SYSCALL(__NR_stat64, path, &lstat);
#else
	ret = LINUX_SYSCALL(__NR_stat, path, &lstat);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	stat_linux_to_bsd64(&lstat, stat);

	return 0;
}

