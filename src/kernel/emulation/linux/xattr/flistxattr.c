#include "flistxattr.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../stat/common.h"
#include "../fdpath.h"
#include <sys/stat.h>
#include "../simple.h"

#ifdef __NR_fstat64
	#define STAT_CALL __NR_fstat64
#else
	#define STAT_CALL __NR_fstat
#endif
#ifndef MAXPATHLEN
#	define MAXPATHLEN 1024
#endif

long sys_flistxattr(int fd, char* namebuf, unsigned long size, int options)
{
	int ret;

	struct linux_stat st;
	ret = LINUX_SYSCALL(STAT_CALL, fd, &st);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	if (S_ISLNK(st.st_mode)) {
		// for links, we need to use `llistxattr`
		// so translate the fd to a Linux path
		char buf[64] = {0};
		char path[4096] = {0};
		__simple_sprintf(buf, "/proc/self/fd/%d", fd);
		ret = LINUX_SYSCALL(__NR_readlink, buf, path, sizeof(path) - 1);
		if (ret < 0)
			return errno_linux_to_bsd(ret);
		path[ret] = '\0';
		ret = LINUX_SYSCALL(__NR_llistxattr, path, namebuf, size);
	} else {
		ret = LINUX_SYSCALL(__NR_flistxattr, fd, namebuf, size);
	}

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return ret;
}

