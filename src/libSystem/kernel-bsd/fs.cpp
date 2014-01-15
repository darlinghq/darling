#include "config.h"
#include "fs.h"
#include <cstring>
#include "common/path.h"
#include "common/auto.h"
#include "libc/errno.h"
#include "libc/darwin_errno_codes.h"
#include <mntent.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/fs.h>
#include "log.h"

template<typename Out> void StatfsLinuxToDarwinGen(const char* path, const struct statvfs* in, Out* out)
{
	out->f_bsize = in->f_bsize;
	out->f_blocks = out->f_iosize = in->f_blocks;
	out->f_bfree = in->f_bfree;
	out->f_bavail = in->f_bavail;
	out->f_files = in->f_files;
	out->f_ffree = in->f_ffree;
	out->f_fsid.val[0] = in->f_fsid;
	
	if (in->f_flag & ST_RDONLY)
		out->f_flags |= DARWIN_MNT_RDONLY;
	if (in->f_flag & ST_NOSUID)
		out->f_flags |= DARWIN_MNT_NOSUID;
	
	struct mntent* ent = findMountForPath(path);
	if (ent)
	{
		strncpy(out->f_mntfromname, ent->mnt_fsname, sizeof(out->f_mntfromname)-1); // we assume zeroed out, no need to set the last char to NUL
		strncpy(out->f_mntonname, ent->mnt_dir, sizeof(out->f_mntonname)-1);
		strncpy(out->f_fstypename, ent->mnt_fsname, sizeof(out->f_fstypename)-1);
	}
}

template<typename T> int __darwin_statfsGen(const char* path, T* buf)
{
	struct statvfs st;
	int rv;
	
	::memset(buf, 0, sizeof(*buf));
	
	rv = statvfs(path, &st);
	if (rv == -1)
		errnoOut();
	else
		StatfsLinuxToDarwinGen(path, &st, buf);
	
	return rv;
}

int __darwin_statfs(const char* path, struct __darwin_statfs* buf)
{
	return __darwin_statfsGen(translatePathCI(path), buf);
}

int __darwin_statfs64(const char* path, struct __darwin_statfs64* buf)
{
	return __darwin_statfsGen(translatePathCI(path), buf);
}

template <typename T> int __darwin_fstatfsGen(int fd, T* out)
{
	char buf[PATH_MAX], buf2[PATH_MAX];
	sprintf(buf, "/proc/%d/fd/%d", getpid(), fd);
	
	memset(out, 0, sizeof(*out));
	
	if (::readlink(buf, buf2, sizeof buf2) == -1)
	{
		errnoOut();
		return -1;
	}
	
	return __darwin_statfsGen(buf2, out);
}

int __darwin_fstatfs(int fd, struct __darwin_statfs* buf)
{
	return __darwin_fstatfsGen(fd, buf);
}

int __darwin_fstatfs64(int fd, struct __darwin_statfs64* buf)
{
	return __darwin_fstatfsGen(fd, buf);
}

template<typename Stat> int __darwin_getfsstatGen(Stat* buf, int bufsize, int flags)
{
	FILE* f = setmntent("/proc/mounts", "r");
	struct mntent *ent;
	
	if (!f)
	{
		LOG << "Cannot access /proc/mounts!\n";
		errno = DARWIN_ENOENT;
		return -1;
	}
	
	int count = 0;
	while ( (ent = getmntent(f)) != 0 && ((count+1)*sizeof(*buf) < bufsize || !buf) )
	{
		if (buf)
		{
			int rv = __darwin_statfsGen(ent->mnt_dir, buf+count);
			if (rv == -1)
				break;
		}
		count++;
	}
	
	endmntent(f);
	return count;
}

int __darwin_getfsstat(struct __darwin_statfs* buf, int bufsize, int flags)
{
	return __darwin_getfsstatGen(buf, bufsize, flags);
}

int __darwin_getfsstat64(struct __darwin_statfs64* buf, int bufsize, int flags)
{
	return __darwin_getfsstatGen(buf, bufsize, flags);
}

void StatfsLinuxToDarwin(const char* path, const struct statvfs* in, struct __darwin_statfs* out)
{
	StatfsLinuxToDarwinGen(path, in, out);
}

void StatfsLinuxToDarwin(const char* path, const struct statvfs* in, struct __darwin_statfs64* out)
{
	StatfsLinuxToDarwinGen(path, in, out);
}

struct mntent* findMountForPath(const char* path)
{
	FILE* f = setmntent("/proc/mounts", "r");
	struct mntent *ent, *rv = 0;
	
	if (!f)
	{
		LOG << "Cannot access /proc/mounts!\n";
		return 0;
	}
	
	while ((ent = getmntent(f)))
	{
		if (strncmp(ent->mnt_dir, path, strlen(ent->mnt_dir)) == 0)
		{
			rv = ent;
			break;
		}
	}
	
	endmntent(f);
	return rv;
}

#define EXT2_IMMUTABLE_FL		0x00000010
#define EXT2_APPEND_FL			0x00000020
#define EXT2_NODUMP_FL			0x00000040

#define	UF_NODUMP		0x00000001
#define	UF_IMMUTABLE	0x00000002
#define	UF_APPEND		0x00000004

static const Darling::MappedFlag g_fflags[] = {
	{ UF_NODUMP, EXT2_NODUMP_FL}, { UF_IMMUTABLE, EXT2_IMMUTABLE_FL }, { UF_APPEND, EXT2_APPEND_FL},
};

int chflags(const char *path, unsigned int flags)
{
	int rv;
	int fd = ::open(path, O_RDWR);
	if (fd == -1)
	{
		errnoOut();
		return -1;
	}
	
	rv = fchflags(fd, flags);
	::close(fd);
	
	return rv;
}

int fchflags(int fd, unsigned int flags)
{
	int rv;
	
	flags = Darling::flagsDarwinToNative(g_fflags, sizeof(g_fflags)/sizeof(g_fflags[0]), flags);
	rv = ioctl(fd, FS_IOC_SETFLAGS, &flags);
	
	if (rv)
		errnoOut();
	
	return rv;
}

int lchflags(const char *path, unsigned int flags)
{
	return DARWIN_ENOSYS; // not possible on Linux
}


