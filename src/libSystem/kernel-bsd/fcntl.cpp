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
		case DARWIN_F_PREALLOCATE:
		{
			__darwin_fallocate* fa = static_cast<__darwin_fallocate*>(arg);
			fa->allocated = 0;

			// We disregard fa->flags, it's not supported on Linux

			if (fa->mode == DARWIN_F_PEOFPOSMODE) // like SEEK_END
			{
				struct stat st;
				if (fstat(fd, &st) == -1)
				{
					errnoOut();
					return -1;
				}
				fa->offset += st.st_size;
			}
			else if (fa->mode == DARWIN_F_VOLPOSMODE) // like SEEK_CUR
			{
				fa->offset += lseek(fd, 0, SEEK_CUR);
			}
			else
			{
				errno = DARWIN_EINVAL;
				return -1;
			}

			int err = posix_fallocate(fd, fa->offset, fa->length);
			if (err) // doesn't use errno!
			{
				errno = errnoLinuxToDarwin(err);
				return -1;
			}
			else
			{
				fa->allocated = fa->length;
				return 0;
			}
		}
		case DARWIN_F_SETSIZE:
		{
			uint64_t size = uint64_t(arg);

			// Not identical to what F_SETSIZE is supposed to do
			// but it's the best we can do.
			if (ftruncate(fd, size) == -1)
			{
				errnoOut();
				return -1;
			}
			return 0;
		}
		case DARWIN_F_RDADVISE:
		{
			const __darwin_rdadvise* adv = static_cast<__darwin_rdadvise*>(arg);
			int err = posix_fadvise(fd, adv->offset, adv->count, POSIX_FADV_WILLNEED);

			if (err) // doesn't use errno!
			{
				errno = errnoLinuxToDarwin(err);
				return -1;
			}
			else
				return 0;
		}
		case DARWIN_F_RDAHEAD:
		{
			int advice = (arg) ? POSIX_FADV_NORMAL : POSIX_FADV_RANDOM;
			int err = posix_fadvise(fd, 0, 1, advice); // on Linux, the offset and length doesn't matter for these advices

			if (err) // doesn't use errno!
			{
				errno = errnoLinuxToDarwin(err);
				return -1;
			}
			else
				return 0;
		}
		// TODO: other values

		default:
			errno = DARWIN_EINVAL;
			return -1;
	}

	return AutoErrno<int>(fcntl, fd, cmd, arg);
}

