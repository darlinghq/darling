#include "fcntl.h"
#include <fcntl.h>
#include <errno.h>
#include <cstdio>
#include <unistd.h>
#include "../libc/darwin_errno_codes.h"
#include "../common/auto.h"
#include "../common/path.h"
#include "io.h"
#include "../libc/errno.h"

int __darwin_fcntl(int fd, int cmd, void* arg)
{
	switch (cmd)
	{
		case DARWIN_F_DUPFD:
			cmd = F_DUPFD;
			break;
		case DARWIN_F_GETFD: // CLOEXEC is compatible
			cmd = F_GETFD;
			break;
		case DARWIN_F_SETFD:
			cmd = F_SETFD;
			break;
		case DARWIN_F_GETFL:
		{
			int value = fcntl(fd, F_GETFL);
			if (value == -1)
			{
				errnoOut();
				return -1;
			}
			else
			{
				return Darling::openflagsNativeToDarwin(value);
			}
			break;
		}
		case DARWIN_F_SETFL:
		{
			int value = (int)(uintptr_t) arg;
			value = Darling::openflagsDarwinToNative(value);
			cmd = F_SETFL;
			arg = (void*) value;
			break;
		}
		case DARWIN_F_GETOWN:
			cmd = F_GETOWN;
			break;
		case DARWIN_F_SETOWN:
			cmd = F_SETOWN;
			break;
		case DARWIN_F_GETPATH:
		{
			char procpath[255];
			int rv;

			sprintf(procpath, "/proc/%d/fd/%d", getpid(), fd);
			rv = readlink(procpath, (char*) arg, DARWIN_MAXPATHLEN);
			
			if (rv == -1)
			{
				errnoOut();
				return -1;
			}
			((char*) arg)[rv] = 0;
			return 0;
		}
		// TODO: other values

		default:
			errno = DARWIN_EINVAL;
			return -1;
	}

	return AutoErrno<int>(fcntl, fd, cmd, arg);
}

