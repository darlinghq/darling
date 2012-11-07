#ifndef BSD_STAT_H
#define BSD_STAT_H
#include <stdint.h>
#include <sys/types.h>

struct __darwin_timespec {
  time_t tv_sec;
  long tv_nsec;
};

typedef int32_t __darwin_dev_t;
typedef uint16_t __darwin_mode_t;
typedef uint16_t __darwin_nlink_t;
typedef uint64_t __darwin_ino64_t;
typedef uint32_t __darwin_uid_t;
typedef uint32_t __darwin_gid_t;

// target: current (INODE64)
// sizeof() = 144 on x86_64
// sizeof() = 108 on x86

// target: legacy (10.3)
// sizeof() = 120 on x86_64
// sizeof() = 96 on x86
// 92

// From /usr/include/sys/stat.h
struct __darwin_stat64 {
  __darwin_dev_t st_dev;
  __darwin_mode_t st_mode;
  __darwin_nlink_t st_nlink;
  __darwin_ino64_t st_ino;
  __darwin_uid_t st_uid;
  __darwin_gid_t st_gid;
  __darwin_dev_t st_rdev;
  struct __darwin_timespec st_atimespec;
  struct __darwin_timespec st_mtimespec;
  struct __darwin_timespec st_ctimespec;
  struct __darwin_timespec st_birthtimespec;
  uint64_t st_size;
  uint64_t st_blocks;
  uint32_t st_blksize;
  __uint32_t st_flags;
  __uint32_t st_gen;
  __int32_t st_lspare;
  __int64_t st_qspare[2];
};

struct __darwin_stat {
  __darwin_dev_t st_dev;
  uint32_t st_ino;
  __darwin_mode_t st_mode;
  __darwin_nlink_t st_nlink;
  __darwin_uid_t st_uid;
  __darwin_gid_t st_gid;
  __darwin_dev_t st_rdev;
  struct __darwin_timespec st_atimespec;
  struct __darwin_timespec st_mtimespec;
  struct __darwin_timespec st_ctimespec;
  uint64_t st_size;
  uint64_t st_blocks;
  uint32_t st_blksize;
  __uint32_t st_flags;
  __uint32_t st_gen;
  __int32_t st_lspare;
  __int64_t st_qspare[2];
};

#ifdef __cplusplus
extern "C"
{
#endif

int __darwin_stat64(const char* path, struct __darwin_stat64* mac) asm("__darwin_stat$INODE64");
int __darwin_fstat64(int fd, struct __darwin_stat64* mac) asm("__darwin_fstat$INODE64");
int __darwin_lstat64(const char* path, struct __darwin_stat64* mac) asm("__darwin_lstat$INODE64");

int __darwin_stat(const char* path, struct __darwin_stat* mac);
int __darwin_fstat(int fd, struct __darwin_stat* mac);
int __darwin_lstat(const char* path, struct __darwin_stat* mac);

#ifdef __cplusplus
}
#endif

#endif
