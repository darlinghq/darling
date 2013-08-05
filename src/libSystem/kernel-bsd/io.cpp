#include "config.h"
#include "io.h"
#include "log.h"
#include "trace.h"
#include "libc/errno.h"
#include "libc/darwin_errno_codes.h"
#include "common/path.h"
#include "common/auto.h"
#include <limits.h>
#include <errno.h>
#include <dyld/MachOMgr.h>

static const Darling::MappedFlag g_openflags[] = {
	{ DARWIN_O_ASYNC, O_ASYNC}, { DARWIN_O_SYNC, O_SYNC }, { DARWIN_O_NOFOLLOW, O_NOFOLLOW},
	{ DARWIN_O_CREAT, O_CREAT }, { DARWIN_O_TRUNC, O_TRUNC }, { DARWIN_O_EXCL, O_EXCL },
	{ DARWIN_O_NOCTTY, O_NOCTTY }, { DARWIN_O_DIRECTORY, O_DIRECTORY }, { DARWIN_O_DSYNC, O_DSYNC },
};

int Darling::openflagsDarwinToNative(int flags)
{
	return Darling::flagsDarwinToNative(g_openflags, sizeof(g_openflags)/sizeof(g_openflags[0]), flags);
}

int Darling::openflagsNativeToDarwin(int flags)
{
	return Darling::flagsNativeToDarwin(g_openflags, sizeof(g_openflags)/sizeof(g_openflags[0]), flags);
}

int __darwin_open(const char* path, int flags, mode_t mode)
{
	TRACE3(path,flags,mode);
	
	int linux_flags = 0;
	linux_flags |= flags & O_ACCMODE;

	if (flags & ( DARWIN_O_EVTONLY|DARWIN_O_SYMLINK|DARWIN_O_POPUP|DARWIN_O_ALERT))
	{
		LOG << "Unsupported open flags: " << flags << std::endl;
		errno = DARWIN_EINVAL;
		return -1;
	}
	linux_flags |= Darling::flagsDarwinToNative(g_openflags, sizeof(g_openflags)/sizeof(g_openflags[0]), flags);
	
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
	
	// Apply sysroot
	//std::cout << "File: " << path << std::endl;
	//std::cout << "Sysroot: " << g_sysroot << std::endl;
	// std::cout << "Flag: " << (linux_flags & O_RDONLY) << std::endl;
	if ( (linux_flags & O_ACCMODE) == 0 && Darling::MachOMgr::instance()->hasSysRoot())
	{
		const char* prefixed;
		std::string lpath = Darling::MachOMgr::instance()->sysRoot();
		
		lpath += '/';
		lpath += path;
		
		prefixed = translatePathCI(lpath.c_str());
		
		if (::access(prefixed, F_OK) == 0)
			path = prefixed;
		else
			path = translatePathCI(path);
	}
	else
		path = translatePathCI(path);

	int rv = open(path, linux_flags, mode);
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

