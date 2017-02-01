#ifndef _COMMON_AT_H
#define _COMMON_AT_H
// Common declarations for the *at family of system calls

#define LINUX_AT_FDCWD	-100
#define LINUX_AT_SYMLINK_NOFOLLOW	0x100
#define LINUX_AT_REMOVEDIR			0x200
#define LINUX_AT_SYMLINK_FOLLOW		0x400

#define BSD_AT_FDCWD	-2
#define BSD_AT_SYMLINK_NOFOLLOW		0x20
#define BSD_AT_REMOVEDIR			0x80
#define BSD_AT_SYMLINK_FOLLOW		0x40

int atflags_bsd_to_linux(int flags);

static inline int atfd(int fd)
{
	if (fd == BSD_AT_FDCWD)
		return LINUX_AT_FDCWD;
	return fd;
}

#endif

