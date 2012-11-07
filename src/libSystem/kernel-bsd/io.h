#ifndef BSD_IO_H
#define BSD_IO_H
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DARWIN_O_EXLOCK 0x20
#define DARWIN_O_SHLOCK 0x10
#define DARWIN_O_ASYNC 0x40
#define DARWIN_O_SYNC 0x80
#define DARWIN_O_NOFOLLOW 0x100
#define DARWIN_O_CREAT 0x200
#define DARWIN_O_TRUNC 0x400
#define DARWIN_O_EXCL 0x800
#define DARWIN_O_EVTONLY 0x8000
#define DARWIN_O_NOCTTY 0x20000
#define DARWIN_O_DIRECTORY 0x100000
#define DARWIN_O_SYMLINK 0x200000
#define DARWIN_O_DSYNC 0x400000
#define DARWIN_O_POPUP 0x80000000
#define DARWIN_O_ALERT 0x20000000

#ifdef __cplusplus
extern "C"
{
#endif

int __darwin_open(const char* path, int flags, mode_t mode);
int __darwin_creat(const char *pathname, mode_t mode);
int __darwin_close(int fd);
int __darwin_fsync(int fd);
int __darwin_fdatasync(int fd);

#ifdef __cplusplus
}

namespace Darling
{
	int openflagsDarwinToNative(int flags);
	int openflagsNativeToDarwin(int flags);
}
#endif

#endif
