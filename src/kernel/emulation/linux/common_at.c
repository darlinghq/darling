#include "common_at.h"

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

