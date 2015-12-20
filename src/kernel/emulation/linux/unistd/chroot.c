#include "chroot.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <libdyld/VirtualPrefix.h>

long sys_chroot(const char* path)
{
	int ret;

	// TODO: should we support chroot with virtual prefixes?

	ret = LINUX_SYSCALL(__NR_chroot, __prefix_translate_path(path));
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

