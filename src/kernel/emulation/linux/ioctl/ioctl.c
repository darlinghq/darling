#include "ioctl.h"
#include "../base.h"
#include "../unistd/readlink.h"
#include "../../../../../platform-include/sys/errno.h"
#include <stddef.h>
#include <stdbool.h>
#include "termios.h"

#define	D_TAPE	1
#define	D_DISK	2
#define	D_TTY	3

#define IOCTL_STEP(x) { int state, retval; state = (x); \
	if (state == IOCTL_HANDLED) return retval; }

static int handle_filio(int fd, int cmd, void* arg, int* retval);

// Emulated ioctl implementation
VISIBLE
long ioctl(int fd, int cmd, void* arg)
{
	IOCTL_STEP(handle_filio(fd, cmd, arg, &retval));
	IOCTL_STEP(handle_termios(fd, cmd, arg, &retval));
	
run_ioctl:
	return __real_ioctl(fd, cmd, arg);
}

extern int sprintf(char* buf, const char* fmt, ...);
extern int strncmp(const char *, const char *, __SIZE_TYPE__);
static bool get_fd_path(int fd, char* buf, size_t len)
{
	char proc[32];
	int ret;

	sprintf(proc, "/proc/self/fd/%d", fd);
	ret = sys_readlink(proc, buf, len-1);

	if (ret <= 0)
		return false;

	buf[ret] = 0;
	return true;
}

static int handle_filio(int fd, int cmd, void* arg, int* retval)
{
	switch (cmd)
	{
		case BSD_FIODTYPE:
		{
			char orig_path[256];
			int* iarg = (int*) arg;

			if (iarg == NULL)
			{
				*retval = -EFAULT;
				return IOCTL_HANDLED;
			}

			if (!get_fd_path(fd, orig_path, sizeof(orig_path)))
			{
				*retval = -EBADF;
				return IOCTL_HANDLED;
			}

			*retval = 0;
			if (strncmp(orig_path, "/dev/pts", 8) == 0
					|| strncmp(orig_path, "/dev/tty", 8) == 0)
			{
				*iarg = D_TTY;
			}
			else if (strncmp(orig_path, "/dev/st", 7) == 0
					|| strncmp(orig_path, "/dev/nst", 8) == 0)
			{
				*iarg = D_TAPE;
			}
			else
			{
				*iarg = D_DISK;
			}
			return IOCTL_HANDLED;
		}
		default:
			return IOCTL_PASS;
	}
}


