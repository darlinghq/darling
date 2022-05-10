#include "openat.h"
#include "open.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../../../../external/libc/include/fcntl.h"
#include "../common_at.h"
#include "../simple.h"
#include "../vchroot_expand.h"
#include "../bsdthread/cancelable.h"

#include <darlingserver/rpc.h>

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
extern char* strcpy(char* dst, const char* src);

long sys_openat(int fd, const char* filename, int flags, unsigned int mode)
{
	CANCELATION_POINT();
	return sys_openat_nocancel(fd, filename, flags, mode);
}

long sys_openat_nocancel(int fd, const char* filename, int flags, unsigned int mode)
{
	int ret, linux_flags;
	if (!filename)
		return -EFAULT;

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
	else if (strcmp(filename, "/dev/console") == 0) {
		// redirect console output to darlingserver
		int err = dserver_rpc_console_open(&ret);
		if (err < 0) {
			__simple_printf("dserver_rpc_console_open failed internally: %d", err);
			__simple_abort();
		}
		if (err > 0) {
			ret = errno_linux_to_bsd(-err);
		}
		return ret;
	}

	struct vchroot_expand_args vc;
	vc.flags = (flags & BSD_O_SYMLINK || flags & BSD_O_NOFOLLOW) ? 0 : VCHROOT_FOLLOW;
	vc.dfd = atfd(fd);

	strcpy(vc.path, filename);
	ret = vchroot_expand(&vc);
	if (ret < 0) {
		return errno_linux_to_bsd(ret);
	}

	// when we're given O_SYMLINK, `oflags_bsd_to_linux` translates it into
	// O_NOFOLLOW and O_PATH
	// this is the only way to open the symlink itself on Linux
	// unfortunately, this presents additional challenges
	// only a select few Linux syscalls support O_PATH descriptors, which means
	// that for other syscalls, we either have to check for this and use an
	// `l` variant (e.g. `llistxattr` instead of `listxatrr`) or we're screwed

	ret = LINUX_SYSCALL(__NR_openat, vc.dfd, vc.path, linux_flags, mode);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
