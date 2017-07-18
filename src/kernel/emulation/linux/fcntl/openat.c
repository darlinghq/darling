#include "openat.h"
#include "open.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../../../../libc/include/fcntl.h"
#include "../common_at.h"

#ifndef O_NOFOLLOW
#	define O_NOFOLLOW 0x0100
#endif
#ifndef O_CLOEXEC
#	define O_CLOEXEC 0x1000000
#endif
#ifndef O_DIRECTORY
#	define O_DIRECTORY 0x100000
#endif

extern int strcmp(const char *s1, const char *s2);

long sys_openat(int fd, const char* filename, int flags, unsigned int mode)
{
	return sys_openat_nocancel(fd, filename, flags, mode);
}

long sys_openat_nocancel(int fd, const char* filename, int flags, unsigned int mode)
{
	int ret, linux_flags;

	linux_flags = oflags_bsd_to_linux(flags);

	if (sizeof(void*) == 4)
	{
		linux_flags |= LINUX_O_LARGEFILE;
	}

	// XNU /dev/random behaves like Linux /dev/urandom
	if (strcmp(filename, "/dev/random") == 0)
		filename = "/dev/urandom";
	// Expected by launchd and CF
	else if (strcmp(filename, "/dev/autofs_nowait") == 0)
		filename = "/dev/null";

	ret = LINUX_SYSCALL(__NR_openat, atfd(fd), filename, linux_flags, mode);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
