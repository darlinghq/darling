#ifndef BSD_FCNTL_H
#define BSD_FCNTL_H
#include <stdint.h>
#include <sys/types.h>

#define DARWIN_F_DUPFD 0
#define DARWIN_F_DUPFD_CLOEXEC 67
#define DARWIN_F_GETFD 1
#define DARWIN_F_SETFD 2
#define DARWIN_F_GETFL 3
#define DARWIN_F_SETFL 4
#define DARWIN_F_GETOWN 5
#define DARWIN_F_SETOWN 6
#define DARWIN_F_GETLK 7
#define DARWIN_F_SETLK 8
#define DARWIN_F_SETLKW 9
#define DARWIN_F_GETPATH 50
#define DARWIN_F_PREALLOCATE 42
#define DARWIN_F_SETSIZE 43
#define DARWIN_F_RDADVISE 44
#define DARWIN_F_RDAHEAD 45
#define DARWIN_F_READBOOTSTRAP 46
#define DARWIN_F_WRITEBOOTSTRAP 47
#define DARWIN_F_NOCACHE 48
#define DARWIN_F_LOG2PHYS 49
#define DARWIN_F_LOG2PHYS_EXT 65
#define DARWIN_F_FULLFSYNC 51
#define DARWIN_F_SETNOSIGPIPE 73
#define DARWIN_F_GETNOSIGPIPE 74

#define DARWIN_FD_CLOEXEC 1
#define DARWIN_F_RDLCK 1
#define DARWIN_F_UNLCK 2
#define DARWIN_F_WRLCK 3

#define DARWIN_F_PEOFPOSMODE 3
#define DARWIN_F_VOLPOSMODE 4 

#define DARWIN_O_NONBLOCK 4

struct __darwin_flock
{
	int64_t l_start;
	int64_t l_len;
	pid_t l_pid;
	short l_type;
	short l_whence;
};

struct __darwin_fallocate
{
	uint32_t flags;
	int mode;
	uint64_t offset, length;
	uint64_t allocated;
};

struct __darwin_rdadvise
{
	uint64_t offset;
	int count;
};

#ifdef __cplusplus
static_assert(sizeof(__darwin_flock) == 24, "Invalid __darwin_flock size");
extern "C" {
#endif

int __darwin_fcntl(int fd, int cmd, void* arg);

#ifdef __cplusplus
}
#endif

#endif

