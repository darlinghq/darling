#include "fcntl.h"
#include "open.h"
#include "../base.h"
#include "../simple.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <sys/errno.h>
#include "../bsdthread/cancelable.h"
#include "../fdpath.h"

#ifndef O_NOFOLLOW
#   define O_NOFOLLOW 0x0100
#endif
#ifndef O_DIRECTORY
#   define O_DIRECTORY 0x100000
#endif
#ifndef MAXPATHLEN
#	define MAXPATHLEN 1024
#endif

int oflags_bsd_to_linux(int flags);
int oflags_linux_to_bsd(int flags);

extern int sprintf(char *str, const char *format, ...);
long sys_readlink(const char* path, char* buf, unsigned long bsize);

static short int flock_type_linux_to_bsd(short int linux) {
	switch (linux) {
		case LINUX_F_RDLCK: return F_RDLCK;
		case LINUX_F_WRLCK: return F_WRLCK;
		case LINUX_F_UNLCK: return F_UNLCK;
		default:            return -1;
	};
};

static short int flock_type_bsd_to_linux(short int bsd) {
	switch (bsd) {
		case F_RDLCK: return LINUX_F_RDLCK;
		case F_WRLCK: return LINUX_F_WRLCK;
		case F_UNLCK: return LINUX_F_UNLCK;
		default:      return -1;
	};
};

static int cmd_bsd_to_linux(int bsd) {
	switch (bsd) {
		case F_DUPFD:  return LINUX_F_DUPFD;
		case F_GETFD:  return LINUX_F_GETFD;
		case F_SETFD:  return LINUX_F_SETFD;
		case F_GETFL:  return LINUX_F_GETFL;
		case F_SETFL:  return LINUX_F_SETFL;
		case F_GETOWN: return LINUX_F_GETOWN;
		case F_SETOWN: return LINUX_F_SETOWN;
		case F_SETLK:  return LINUX_F_SETLK;
		case F_SETLKW: return LINUX_F_SETLKW;
		case F_GETLK:  return LINUX_F_GETLK;
		case F_DUPFD_CLOEXEC: return LINUX_F_DUPFD_CLOEXEC;
		default:       return -1;
	};
};

long sys_fcntl(int fd, int cmd, long arg)
{
	CANCELATION_POINT();
	return sys_fcntl_nocancel(fd, cmd, arg);
}

long sys_fcntl_nocancel(int fd, int cmd, long arg)
{
	int ret, linux_cmd = cmd_bsd_to_linux(cmd);
	long original_arg = arg;

	switch (cmd)
	{
		case F_CHECK_LV:
			return 0;
		case F_SETFD:
			if (arg & ~FD_CLOEXEC)
				return -EINVAL;
			break;
		case F_SETFL:
			arg = oflags_bsd_to_linux(arg);
			break;
		case F_GETPATH:
		{
			ret = fdpath(fd, arg, MAXPATHLEN);
			if (ret < 0)
				return errno_linux_to_bsd(ret);

			return 0;
		}
		case F_SETLK:
		case F_SETLKW:
		case F_GETLK: {
			struct bsd_flock* bsd_struct = arg;
			struct linux_flock* linux_struct = __builtin_alloca(sizeof(struct linux_flock));
			linux_struct->l_type = flock_type_bsd_to_linux(bsd_struct->l_type);
			linux_struct->l_whence = bsd_struct->l_whence;
			linux_struct->l_start = bsd_struct->l_start;
			linux_struct->l_len = bsd_struct->l_len;
			linux_struct->l_pid = bsd_struct->l_pid;
			arg = linux_struct;
		} break;
		case F_FULLFSYNC: {
			ret = LINUX_SYSCALL1(__NR_fsync, fd);
			if (ret < 0)
				ret = errno_linux_to_bsd(ret);
			return 0;
		};
		// TODO: implement remaining commands
		default:
			if (linux_cmd == -1)
				return -EINVAL;
	}

#ifdef __NR_fcntl64
	ret = LINUX_SYSCALL(__NR_fcntl64, fd, linux_cmd, arg);
#else
	ret = LINUX_SYSCALL(__NR_fcntl, fd, linux_cmd, arg);
#endif
	if (ret < 0)
	{
		ret = errno_linux_to_bsd(ret);
		return ret;
	}

	switch (cmd)
	{
		case F_GETFL:
			ret = oflags_linux_to_bsd(ret);
			break;
		case F_SETLK:
		case F_SETLKW:
		case F_GETLK: {
			struct bsd_flock* bsd_struct = original_arg;
			struct linux_flock* linux_struct = arg;
			bsd_struct->l_type = flock_type_linux_to_bsd(linux_struct->l_type);
			bsd_struct->l_whence = linux_struct->l_whence;
			bsd_struct->l_start = linux_struct->l_start;
			bsd_struct->l_len = linux_struct->l_len;
			bsd_struct->l_pid = linux_struct->l_pid;
		} break;
	}

	return ret;
}

