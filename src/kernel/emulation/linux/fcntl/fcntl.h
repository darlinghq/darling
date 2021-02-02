#ifndef LINUX_FCNTL_H
#define LINUX_FCNTL_H

long sys_fcntl(int fd, int cmd, long arg);
long sys_fcntl_nocancel(int fd, int cmd, long arg);

enum {
	LINUX_F_DUPFD = 0,
	LINUX_F_GETFD,
	LINUX_F_SETFD,
	LINUX_F_GETFL,
	LINUX_F_SETFL,
	LINUX_F_GETLK,
	LINUX_F_SETLK,
	LINUX_F_SETLKW,
	LINUX_F_SETOWN,
	LINUX_F_GETOWN,
	LINUX_F_SETSIG,
	LINUX_F_GETSIG,
	LINUX_F_GETLK64,
	LINUX_F_SETLK64,
	LINUX_F_SETLKW64,
	LINUX_F_SETOWN_EX,
	LINUX_F_GETOWN_EX,
	LINUX_F_GETOWNER_UIDS,
	LINUX_F_DUPFD_CLOEXEC = 1030,
};

enum {
	F_DUPFD = 0,
	F_GETFD,
	F_SETFD,
	F_GETFL,
	F_SETFL,
	F_GETOWN,
	F_SETOWN,
	F_GETLK,
	F_SETLK,
	F_SETLKW,
	F_SETLKWTIMEOUT,
	F_FLUSH_DATA = 40,
	F_CHKCLEAN,
	F_PREALLOCATE,
	F_SETSIZE,
	F_RDADVISE,
	F_RDAHEAD,
	F_GETPATH = 50,
	F_FULLFSYNC = 51,
	F_DUPFD_CLOEXEC = 67,
	F_CHECK_LV = 98,
};

enum {
	LINUX_F_RDLCK = 0,
	LINUX_F_WRLCK,
	LINUX_F_UNLCK,
};

enum {
	F_RDLCK = 1,
	F_UNLCK,
	F_WRLCK,
};

#ifndef FD_CLOEXEC
#   define FD_CLOEXEC 1
#endif

struct bsd_flock {
	long long l_start;
	long long l_len;
	int l_pid;
	short l_type;
	short l_whence;
};

struct linux_flock {
	short int l_type;
	short int l_whence;
	long int l_start;
	long int l_len;
	int l_pid;
};

#endif

