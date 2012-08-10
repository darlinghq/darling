#include "config.h"
#include "io.h"
#include "log.h"
#include "trace.h"
#include "libc/errno.h"
#include "libc/darwin_errno_codes.h"
#include "common/path.h"
#include "common/auto.h"
#include <errno.h>

int __darwin_open(const char* path, int flags, mode_t mode)
{
	TRACE3(path,flags,mode);
	
	int linux_flags = 0;
	linux_flags |= flags & O_ACCMODE;
	
	if (flags & DARWIN_O_SHLOCK)
	{
		errno = DARWIN_EINVAL;
		fprintf(stderr, "Unsupported open flag=%d\n", flags);
		return -1;
	}
	
	if (flags & DARWIN_O_EXLOCK)
	{
		fprintf(stderr, "Unsupported open flag=%d\n", flags);
		errno = DARWIN_EINVAL;
		return -1;
	}
	if (flags & DARWIN_O_ASYNC) linux_flags |= O_ASYNC;
	if (flags & DARWIN_O_SYNC) linux_flags |= O_SYNC;
	if (flags & DARWIN_O_NOFOLLOW) linux_flags |= O_NOFOLLOW;
	if (flags & DARWIN_O_CREAT) linux_flags |= O_CREAT;
	if (flags & DARWIN_O_TRUNC) linux_flags |= O_TRUNC;
	if (flags & DARWIN_O_EXCL) linux_flags |= O_EXCL;
	
	if (flags & DARWIN_O_EVTONLY)
	{
		errno = DARWIN_EINVAL;
		fprintf(stderr, "Unsupported open flag=%d\n", flags);
		return -1;
	}
	if (flags & DARWIN_O_NOCTTY) linux_flags |= O_NOCTTY;
	if (flags & DARWIN_O_DIRECTORY) linux_flags |= O_DIRECTORY;
	
	if (flags & DARWIN_O_SYMLINK)
	{
		errno = DARWIN_EINVAL;
		fprintf(stderr, "Unsupported open flag=%d\n", flags);
		return -1;
	}
	if (flags & DARWIN_O_DSYNC) linux_flags |= O_DSYNC;
	
	if (flags & DARWIN_O_POPUP)
	{
		errno = DARWIN_EINVAL;
		fprintf(stderr, "Unsupported open flag=%d\n", flags);
		return -1;
	}
	
	if (flags & DARWIN_O_ALERT)
	{
		errno = DARWIN_EINVAL;
		fprintf(stderr, "Unsupported open flag=%d\n", flags);
		return -1;
	}

	int rv = open(translatePathCI(path), linux_flags, mode);
	if (rv == -1)
		errnoOut();
	return rv;
}

int __darwin_creat(const char *pathname, mode_t mode)
{
	return AutoPathErrno<int>(creat, pathname, mode);
}

MAP_FUNCTION1(int,close,int);
MAP_FUNCTION1(int,fsync,int);
MAP_FUNCTION1(int,fdatasync,int);
MAP_FUNCTION3(ssize_t,read,int,void*,size_t);
MAP_FUNCTION3(ssize_t,write,int,const void*,size_t);

off64_t __darwin_lseek(int fd, off64_t offset, int whence)
{
	off64_t rv = lseek64(fd, offset, whence);
	if (rv == -1)
		errnoOut();
	return rv;
}

MAP_FUNCTION1(int,dup,int);
MAP_FUNCTION2(int,dup2,int,int);

