#include "chmod.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <libdyld/VirtualPrefix.h>

long sys_chmod(const char* path, int mode)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_chmod, __prefix_translate_path(path), mode);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

