#include <sys/stat.h>
#include "common.h"
#include "../unistd/getuid.h"
#include "../unistd/getgid.h"

void stat_linux_to_bsd(const struct linux_stat* lstat, struct stat* stat)
{
	stat->st_dev = lstat->st_dev;
	stat->st_mode = lstat->st_mode;
	stat->st_nlink = lstat->st_nlink;
	stat->st_ino = lstat->st_ino;
	stat->st_uid = /*lstat->st_uid*/ sys_getuid();
	stat->st_gid = /*lstat->st_gid*/ sys_getgid();
	stat->st_rdev = lstat->st_rdev;
	stat->st_size = lstat->st_size;
	stat->st_blksize = lstat->st_blksize;
	stat->st_blocks = lstat->st_blocks;
	stat->st_atimespec.tv_sec = lstat->st_atime;
	stat->st_atimespec.tv_nsec = lstat->st_atime_nsec;
	stat->st_mtimespec.tv_sec = lstat->st_mtime;
	stat->st_mtimespec.tv_nsec = lstat->st_mtime_nsec;
	stat->st_ctimespec.tv_sec = lstat->st_ctime;
	stat->st_ctimespec.tv_nsec = lstat->st_ctime_nsec;
	stat->st_flags = 0;
}

void stat_linux_to_bsd64(const struct linux_stat* lstat, struct stat64* stat)
{
	stat->st_dev = lstat->st_dev;
	stat->st_mode = lstat->st_mode;
	stat->st_nlink = lstat->st_nlink;
	stat->st_ino = lstat->st_ino;
	stat->st_uid = /*lstat->st_uid*/ sys_getuid();
	stat->st_gid = /*lstat->st_gid*/ sys_getgid();
	stat->st_rdev = lstat->st_rdev;
	stat->st_size = lstat->st_size;
	stat->st_blksize = lstat->st_blksize;
	stat->st_blocks = lstat->st_blocks;
	stat->st_atimespec.tv_sec = lstat->st_atime;
	stat->st_atimespec.tv_nsec = lstat->st_atime_nsec;
	stat->st_mtimespec.tv_sec = lstat->st_mtime;
	stat->st_mtimespec.tv_nsec = lstat->st_mtime_nsec;
	stat->st_ctimespec.tv_sec = lstat->st_ctime;
	stat->st_ctimespec.tv_nsec = lstat->st_ctime_nsec;
	stat->st_flags = 0;
}

void statfs_linux_to_bsd(const struct linux_statfs64* lstat, struct bsd_statfs* stat)
{
	stat->f_type = lstat->f_type;
	stat->f_bsize = lstat->f_bsize;
	stat->f_blocks = lstat->f_blocks;
	stat->f_bfree = lstat->f_bfree;
	stat->f_bavail = lstat->f_bavail;
	stat->f_files = lstat->f_files;
	stat->f_ffree = lstat->f_ffree;
	stat->f_fsid = lstat->f_fsid;
	stat->f_flags = lstat->f_flags; /* FIXME: convert flags */
}

void statfs_linux_to_bsd64(const struct linux_statfs64* lstat, struct bsd_statfs64* stat)
{
	stat->f_type = lstat->f_type;
	stat->f_bsize = lstat->f_bsize;
	stat->f_blocks = lstat->f_blocks;
	stat->f_bfree = lstat->f_bfree;
	stat->f_bavail = lstat->f_bavail;
	stat->f_files = lstat->f_files;
	stat->f_ffree = lstat->f_ffree;
	stat->f_fsid = lstat->f_fsid;
	stat->f_flags = lstat->f_flags; /* FIXME: convert flags */
}

