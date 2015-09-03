#include "../../../../platform-include/sys/stat.h"
#include "common.h"

void stat_linux_to_bsd(const struct linux_stat* lstat, struct stat* stat)
{
	stat->st_dev = lstat->st_dev;
	stat->st_mode = lstat->st_mode;
	stat->st_nlink = lstat->st_nlink;
	stat->st_ino = lstat->st_ino;
	stat->st_uid = lstat->st_uid;
	stat->st_gid = lstat->st_gid;
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
}

void stat_linux_to_bsd64(const struct linux_stat* lstat, struct stat64* stat)
{
	stat->st_dev = lstat->st_dev;
	stat->st_mode = lstat->st_mode;
	stat->st_nlink = lstat->st_nlink;
	stat->st_ino = lstat->st_ino;
	stat->st_uid = lstat->st_uid;
	stat->st_gid = lstat->st_gid;
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
}

