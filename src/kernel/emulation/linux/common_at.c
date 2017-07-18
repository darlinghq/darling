#include "common_at.h"
#include "bsdthread/per_thread_wd.h"

int atflags_bsd_to_linux(int flags)
{
	int linux_flags = 0;

	if (flags & BSD_AT_SYMLINK_NOFOLLOW)
		linux_flags |= LINUX_AT_SYMLINK_NOFOLLOW;
	if (flags & BSD_AT_REMOVEDIR)
		linux_flags |= LINUX_AT_REMOVEDIR;
	if (flags & BSD_AT_SYMLINK_FOLLOW)
		linux_flags |= LINUX_AT_SYMLINK_FOLLOW;

	return linux_flags;
}

int atfd(int fd)
{
	if (fd == BSD_AT_FDCWD)
		return get_perthread_wd();
	return fd;
}
