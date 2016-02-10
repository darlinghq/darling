#include "lstat.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <libdyld/VirtualPrefix.h>

long sys_lstat(const char* path, struct stat* stat)
{
	int ret;
	struct linux_stat lstat;

#ifdef __NR_lstat64
	ret = LINUX_SYSCALL(__NR_lstat64, __prefix_translate_path_link(path), &lstat);
#else
	ret = LINUX_SYSCALL(__NR_lstat, __prefix_translate_path_link(path), &lstat);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);
	else if (__prefix_is_system_root(path))
		lstat.st_mode = (lstat.st_mode & ~0120000) | 0040000; // make directory

	stat_linux_to_bsd(&lstat, stat);

	return 0;
}

long sys_lstat64(const char* path, struct stat64* stat)
{
	int ret;
	struct linux_stat lstat;

#ifdef __NR_lstat64
	ret = LINUX_SYSCALL(__NR_lstat64, __prefix_translate_path_link(path), &lstat);
#else
	ret = LINUX_SYSCALL(__NR_lstat, __prefix_translate_path_link(path), &lstat);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);
	else if (__prefix_is_system_root(path))
		lstat.st_mode = (lstat.st_mode & ~0120000) | 0040000; // make directory

	stat_linux_to_bsd64(&lstat, stat);

	return 0;
}

