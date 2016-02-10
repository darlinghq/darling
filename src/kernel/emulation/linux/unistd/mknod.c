#include "mknod.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <libdyld/VirtualPrefix.h>

long sys_mknod(const char* path, int mode, int dev)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_mknod, __prefix_translate_path(path), mode, dev);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

