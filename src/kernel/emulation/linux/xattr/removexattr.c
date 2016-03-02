#include "removexattr.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <libdyld/VirtualPrefix.h>

#define XATTR_NOFOLLOW	1

long sys_removexattr(const char* path, const char* name, int options)
{
	int ret;

	if (options & XATTR_NOFOLLOW)
		ret = LINUX_SYSCALL(__NR_lremovexattr, __prefix_translate_path(path),
				name);
	else
		ret = LINUX_SYSCALL(__NR_removexattr, __prefix_translate_path(path),
				name);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return ret;
}

