#include "config.h"
#include "stat.h"
#include "trace.h"
#include "common/path.h"
#include "libc/errno.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <dyld/MachOMgr.h>

static void convertStat64(struct stat64* linux_buf, struct __darwin_stat64* mac)
{
	mac->st_dev = linux_buf->st_dev;
	mac->st_mode = linux_buf->st_mode;
	mac->st_nlink = linux_buf->st_nlink;
	mac->st_ino = linux_buf->st_ino;
	mac->st_uid = linux_buf->st_uid;
	mac->st_gid = linux_buf->st_gid;
	mac->st_rdev = linux_buf->st_rdev;
	mac->st_size = linux_buf->st_size;
	mac->st_blksize = linux_buf->st_blksize;
	mac->st_blocks = linux_buf->st_blocks;
	mac->st_atimespec.tv_sec = linux_buf->st_atime;
	mac->st_mtimespec.tv_sec = linux_buf->st_mtime;
	mac->st_ctimespec.tv_sec = linux_buf->st_ctime;
}

static void convertStat(struct stat64* linux_buf, struct __darwin_stat* mac)
{
	mac->st_dev = linux_buf->st_dev;
	mac->st_mode = linux_buf->st_mode;
	mac->st_nlink = linux_buf->st_nlink;
	mac->st_ino = linux_buf->st_ino;
	mac->st_uid = linux_buf->st_uid;
	mac->st_gid = linux_buf->st_gid;
	mac->st_rdev = linux_buf->st_rdev;
	mac->st_size = linux_buf->st_size;
	mac->st_blksize = linux_buf->st_blksize;
	mac->st_blocks = linux_buf->st_blocks;
	mac->st_atimespec.tv_sec = linux_buf->st_atime;
	mac->st_mtimespec.tv_sec = linux_buf->st_mtime;
	mac->st_ctimespec.tv_sec = linux_buf->st_ctime;
}

int __darwin_stat64(const char* path, struct __darwin_stat64* mac)
{
	TRACE2(path, mac);
	struct stat64 linux_buf;
	
	if (Darling::MachOMgr::instance()->hasSysRoot())
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
	
	int ret = stat64(translatePathCI(path), &linux_buf);
	if (ret == -1)
		errnoOut();
	
	convertStat64(&linux_buf, mac);
	return ret;
}

int __darwin_fstat64(int fd, struct __darwin_stat64* mac)
{
	TRACE2(fd, mac);
	struct stat64 linux_buf;
	int ret = fstat64(fd, &linux_buf);
	if (ret == -1)
		errnoOut();
	
	convertStat64(&linux_buf, mac);
	return ret;
}

int __darwin_lstat64(const char* path, struct __darwin_stat64* mac)
{
	TRACE2(path, mac);
	struct stat64 linux_buf;
	int ret = lstat64(translatePathCI(path), &linux_buf);
	if (ret == -1)
		errnoOut();
	
	convertStat64(&linux_buf, mac);
	return ret;
}

int __darwin_stat(const char* path, struct __darwin_stat* mac)
{
  TRACE2(path, mac);
  struct stat64 linux_buf;
  int ret = stat64(translatePathCI(path), &linux_buf);
  if (ret == -1)
		errnoOut();
  
  convertStat(&linux_buf, mac);
  return ret;
}

int __darwin_fstat(int fd, struct __darwin_stat* mac)
{
	TRACE2(fd, mac);
	struct stat64 linux_buf;
	int ret = fstat64(fd, &linux_buf);
	if (ret == -1)
		errnoOut();
	
	convertStat(&linux_buf, mac);
	return ret;
}

int __darwin_lstat(const char* path, struct __darwin_stat* mac)
{
	TRACE2(path, mac);
	struct stat64 linux_buf;
	int ret = lstat64(translatePathCI(path), &linux_buf);
	if (ret == -1)
		errnoOut();
	
	convertStat(&linux_buf, mac);
	return ret;
}

#ifdef TEST
int main()
{
	printf("sizeof(__darwin_stat64) = %d\n", sizeof(struct __darwin_stat64));
	printf("sizeof(__darwin_stat) = %d\n", sizeof(struct __darwin_stat));
	return 0;
}

#endif

