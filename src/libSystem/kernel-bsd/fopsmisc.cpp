#include "config.h"
#include "fopsmisc.h"
#include "libc/errno.h"
#include "common/path.h"
#include "common/auto.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <cstdlib>
#include <iostream>
#include <limits.h>
#include <libdyld/MachOMgr.h>

int __darwin_access(const char *pathname, int mode)
{
	//std::cout << "access: " << pathname << std::endl;
	if ((mode & W_OK) == 0 && Darling::MachOMgr::instance()->hasSysRoot())
	{
		// Try to apply a sysroot prefix
		const char* prefixed;
		std::string lpath = Darling::MachOMgr::instance()->sysRoot();
		
		lpath += '/';
		lpath += pathname;
		
		prefixed = translatePathCI(lpath.c_str());
		if (::access(prefixed, F_OK) == 0)
		{
			int rv = ::access(prefixed, mode);
			if (rv == -1)
				errnoOut();
			return rv;
		}
	}
	
	return AutoPathErrno<int>(access, pathname, mode);
}

int __darwin_chown(const char *path, uid_t owner, gid_t group)
{
	return AutoPathErrno<int>(chown, path, owner, group);
}

MAP_FUNCTION3(int,fchown,int,uid_t,gid_t);
MAP_FUNCTION2(int,fchmod,int,mode_t);
MAP_FUNCTION1(int,fchdir,int);

int __darwin_lchown(const char *path, uid_t owner, gid_t group)
{
	return AutoPathErrno<int>(lchown, path, owner, group);
}

int __darwin_chmod(const char *path, mode_t mode)
{
	return AutoPathErrno<int>(chmod, path, mode);
}

ssize_t __darwin_readlink(const char *path, char *buf, size_t bufsiz)
{
	return AutoPathErrno<ssize_t>(readlink, path, buf, bufsiz);
}

template<typename Func> int TwoPathOp(Func f, const char* oldpath, const char* newpath)
{
	char op[DARWIN_MAXPATHLEN];
	char np[DARWIN_MAXPATHLEN];
	
	strcpy(op, oldpath);
	strcpy(np, newpath);
	
	translatePathCI(op);
	translatePathCI(np);
	
	int rv = f(op, np);
	if (rv == -1)
		errnoOut();
	
	return rv;
}

int __darwin_symlink(const char *oldpath, const char *newpath)
{
	return TwoPathOp(symlink, oldpath, newpath);
}

int __darwin_link(const char *oldpath, const char *newpath)
{
	return TwoPathOp(link, oldpath, newpath);
}

int __darwin_unlink(const char *pathname)
{
	return AutoPathErrno<int>(unlink, pathname);
}

int __darwin_rmdir(const char *pathname)
{
	return AutoPathErrno<int>(rmdir, pathname);
}

int __darwin_mknod(const char *pathname, mode_t mode, dev_t dev)
{
	// TODO: check dev_t compatibility
	return AutoPathErrno<int>(mknod, pathname, mode, dev);
}

int __darwin_mkdir(const char *pathname, mode_t mode)
{
	return AutoPathErrno<int>(mkdir, pathname, mode);
}

int __darwin_chdir(const char *path)
{
	if (Darling::MachOMgr::instance()->hasSysRoot())
	{
		// Try to apply a sysroot prefix
		const char* prefixed;
		std::string lpath = Darling::MachOMgr::instance()->sysRoot();
		
		lpath += '/';
		lpath += path;
		
		prefixed = translatePathCI(lpath.c_str());
		if (::chdir(prefixed) == 0)
			return 0;
	}
	
	return AutoPathErrno<int>(chdir, path);
}

int __darwin_acct(const char *filename)
{
	return AutoPathErrno<int>(acct, filename);
}
