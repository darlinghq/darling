#include "open.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
//#include "../../../../platform-include/sys/fcntl.h"
#include "../../../../libc/include/fcntl.h"

#ifndef O_NOFOLLOW
#	define O_NOFOLLOW 0x0100
#endif
#ifndef O_CLOEXEC
#	define O_CLOEXEC 0x1000000
#endif
#ifndef O_DIRECTORY
#	define O_DIRECTORY 0x100000
#endif

int oflags_bsd_to_linux(int flags);

extern int strcmp(const char *s1, const char *s2);

long sys_open(const char* filename, int flags, unsigned int mode)
{
	return sys_open_nocancel(filename, flags, mode);
}

long sys_open_nocancel(const char* filename, int flags, unsigned int mode)
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

	ret = LINUX_SYSCALL(__NR_open, filename, linux_flags, mode);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

int oflags_bsd_to_linux(int flags)
{
	int linux_flags = 0;

	if (flags & BSD_O_RDONLY) /* always false */
		linux_flags |= LINUX_O_RDONLY;
	if (flags & BSD_O_WRONLY)
		linux_flags |= LINUX_O_WRONLY;
	if (flags & BSD_O_RDWR)
		linux_flags |= LINUX_O_RDWR;
	if (flags & BSD_O_NONBLOCK)
		linux_flags |= LINUX_O_NONBLOCK;
	if (flags & BSD_O_APPEND)
		linux_flags |= LINUX_O_APPEND;
	if (flags & BSD_O_CREAT)
		linux_flags |= LINUX_O_CREAT;
	if (flags & BSD_O_TRUNC)
		linux_flags |= LINUX_O_TRUNC;
	if (flags & BSD_O_EXCL)
		linux_flags |= LINUX_O_EXCL;
	if (flags & BSD_O_CLOEXEC)
		linux_flags |= LINUX_O_CLOEXEC;
	if (flags & BSD_O_NOFOLLOW)
		linux_flags |= LINUX_O_NOFOLLOW;
	if (flags & BSD_O_DIRECTORY)
		linux_flags |= LINUX_O_DIRECTORY;
	if (flags & BSD_O_CLOEXEC)
		linux_flags |= LINUX_O_CLOEXEC;
	if (flags & BSD_O_NOCTTY)
		linux_flags |= LINUX_O_NOCTTY;

	return linux_flags;
}

int oflags_linux_to_bsd(int flags)
{
	int bsd_flags = 0;

	if (flags & LINUX_O_RDONLY) /* always false */
		bsd_flags |= BSD_O_RDONLY;
	if (flags & LINUX_O_WRONLY)
		bsd_flags |= BSD_O_WRONLY;
	if (flags & LINUX_O_RDWR)
		bsd_flags |= BSD_O_RDWR;
	if (flags & LINUX_O_NONBLOCK)
		bsd_flags |= BSD_O_NONBLOCK;
	if (flags & LINUX_O_APPEND)
		bsd_flags |= BSD_O_APPEND;
	if (flags & LINUX_O_CREAT)
		bsd_flags |= BSD_O_CREAT;
	if (flags & LINUX_O_TRUNC)
		bsd_flags |= BSD_O_TRUNC;
	if (flags & LINUX_O_EXCL)
		bsd_flags |= BSD_O_EXCL;
	if (flags & LINUX_O_CLOEXEC)
		bsd_flags |= BSD_O_CLOEXEC;
	if (flags & LINUX_O_NOFOLLOW)
		bsd_flags |= BSD_O_NOFOLLOW;
	if (flags & LINUX_O_DIRECTORY)
		bsd_flags |= BSD_O_DIRECTORY;
	if (flags & LINUX_O_CLOEXEC)
		bsd_flags |= BSD_O_CLOEXEC;
	if (flags & LINUX_O_NOCTTY)
		bsd_flags |= BSD_O_NOCTTY;

	return bsd_flags;
}
