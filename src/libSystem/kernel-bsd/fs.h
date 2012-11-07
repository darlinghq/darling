#ifndef BSD_FS_H
#define BSD_FS_H
#include <stdint.h>
#include "common/path.h"

#define MFSNAMELEN		15
#define MFSTYPENAMELEN	16
#define MNAMELEN		90

#define DARWIN_MNT_RDONLY      0x1
#define DARWIN_MNT_SYNCHRONOUS 0x2
#define DARWIN_MNT_NOEXEC      0x4
#define DARWIN_MNT_NOSUID      0x8
#define DARWIN_MNT_NODEV       0x10
#define DARWIN_MNT_UNION       0x20
#define DARWIN_MNT_ASYNC       0x40
#define DARWIN_MNT_CPROTECT    0x80


typedef struct { int32_t val[2]; } darwin_fsid_t;

struct __darwin_statfs64
{
	uint32_t f_bsize;
	int32_t  f_iosize;
	uint64_t f_blocks;
	uint64_t f_bfree;
	uint64_t f_bavail;
	uint64_t f_files;
	uint64_t f_ffree;
	darwin_fsid_t   f_fsid;
	uint32_t    f_owner;
	uint32_t f_type;
	uint32_t f_flags;
	uint32_t f_fssubtype;
	char     f_fstypename[MFSTYPENAMELEN];
	char     f_mntonname[DARWIN_MAXPATHLEN];
	char     f_mntfromname[DARWIN_MAXPATHLEN];
	uint32_t f_reserved[8];
};

struct __darwin_statfs
{
	short   f_otype;
	short   f_oflags;
	long    f_bsize;
	long    f_iosize;
	long    f_blocks;
	long    f_bfree;
	long    f_bavail;
	long    f_files;
	long    f_ffree;
	darwin_fsid_t  f_fsid;
	uint32_t   f_owner;
	short   f_reserved1;
	short   f_type;
	long    f_flags;
	long    f_reserved2[2];
	char    f_fstypename[MFSNAMELEN];
	char    f_mntonname[MNAMELEN];
	char    f_mntfromname[MNAMELEN];
	char    f_reserved3;
	long    f_reserved4[4];
};

#ifdef __cplusplus
extern "C"
{
#endif

int __darwin_statfs(const char* path, struct __darwin_statfs* buf);
int __darwin_statfs64(const char* path, struct __darwin_statfs64* buf) asm("__darwin_statfs$INODE64");
int __darwin_fstatfs(int fd, struct __darwin_statfs* buf);
int __darwin_fstatfs64(int fd, struct __darwin_statfs64* buf) asm("__darwin_fstatfs$INODE64");
int __darwin_getfsstat(struct __darwin_statfs* buf, int bufsize, int flags);
int __darwin_getfsstat64(struct __darwin_statfs64* buf, int bufsize, int flags) asm("__darwin_getfsstat$INODE64");

#ifdef __cplusplus
}

// out assumed to have been zeroed out
void StatfsLinuxToDarwin(const char* path, const struct statvfs* in, struct __darwin_statfs* out);
void StatfsLinuxToDarwin(const char* path, const struct statvfs* in, struct __darwin_statfs64* out);

struct mntent;
struct mntent* findMountForPath(const char* path);
#endif

#endif
