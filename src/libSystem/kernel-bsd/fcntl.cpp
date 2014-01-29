#include "config.h"
#include "fcntl.h"
#include <fcntl.h>
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include "../libc/darwin_errno_codes.h"
#include "../common/auto.h"
#include "../common/path.h"
#include "io.h"
#include "../libc/errno.h"

#define REQUIRE_ARG_PTR(arg) { if (!arg) { errno = DARWIN_EINVAL; return -1; } }

int __darwin_fcntl(int fd, int cmd, void* arg)
{
	switch (cmd)
	{
		case DARWIN_F_DUPFD:
			cmd = F_DUPFD;
			break;
		case DARWIN_F_DUPFD_CLOEXEC:
			cmd = F_DUPFD_CLOEXEC;
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
				int d = Darling::openflagsNativeToDarwin(value);
				if (value & O_NONBLOCK)
					d |= DARWIN_O_NONBLOCK;
				return d;
			}
			break;
		}
		case DARWIN_F_SETFL:
		{
			int orig = (int)(uintptr_t) arg;
			int value;
			value = Darling::openflagsDarwinToNative(value);
			
			if (orig & DARWIN_O_NONBLOCK)
				value |= O_NONBLOCK;
			
			cmd = F_SETFL;
			arg = (void*)(uintptr_t(value) & 0xffffffff);
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
			REQUIRE_ARG_PTR(arg);
			
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
			REQUIRE_ARG_PTR(arg);
			
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
			REQUIRE_ARG_PTR(arg);
			
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
			REQUIRE_ARG_PTR(arg);
			
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
		case DARWIN_F_NOCACHE:
		{
			struct stat st;
			if (fstat(fd, &st) == -1)
			{
				errnoOut();
				return -1;
			}
			
			// This is as close as we can get...
			
			int advice = (arg) ? POSIX_FADV_NORMAL : POSIX_FADV_DONTNEED;
			int err = posix_fadvise(fd, 0, 1, advice);
			
			if (err) // doesn't use errno!
			{
				errno = errnoLinuxToDarwin(err);
				return -1;
			}
			else
				return 0;
		}
		case DARWIN_F_FULLFSYNC:
		{
			if (fsync(fd) == -1)
			{
				errnoOut();
				return -1;
			}
			
			// This is as close as we can get...
			// Linux 2.6.39+
			if (syncfs(fd) == -1)
			{
				errnoOut();
				return -1;
			}
			return 0;
		}
		case DARWIN_F_GETLK:
		{
			REQUIRE_ARG_PTR(arg);
			
			__darwin_flock* lock = static_cast<__darwin_flock*>(arg);
			struct flock native;
			
			memset(lock, 0, sizeof(*lock));
			
			if (fcntl(fd, F_GETLK, &native) == -1)
			{
				errnoOut();
				return -1;
			}
			
			lock->l_start = native.l_start;
			lock->l_len = native.l_len;
			lock->l_pid = native.l_pid;
			lock->l_type = native.l_type;
			lock->l_whence = native.l_whence;
			
			return 0;
		}
		case DARWIN_F_SETLK:
		case DARWIN_F_SETLKW:
		{
			REQUIRE_ARG_PTR(arg);
			
			int ncmd = (cmd == DARWIN_F_SETLK) ? F_SETLK : F_SETLKW;
			const __darwin_flock* lock = static_cast<__darwin_flock*>(arg);
			struct flock native;
			
			native.l_start = lock->l_start;
			native.l_len = lock->l_len;
			native.l_pid = lock->l_pid;
			native.l_type = lock->l_type;
			native.l_whence = lock->l_whence;
			
			return AutoErrno<int>(fcntl, fd, ncmd, &native);
		}
		case DARWIN_F_SETNOSIGPIPE:
		case DARWIN_F_GETNOSIGPIPE:
		case DARWIN_F_READBOOTSTRAP:
		case DARWIN_F_WRITEBOOTSTRAP:
		case DARWIN_F_LOG2PHYS:
		case DARWIN_F_LOG2PHYS_EXT:
		{
			errno = DARWIN_ENOSYS;
			return -1;
		}

		default:
			errno = DARWIN_EINVAL;
			return -1;
	}

	return AutoErrno<int>(fcntl, fd, cmd, arg);
}

