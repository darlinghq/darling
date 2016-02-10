#include "mkdir.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <libdyld/VirtualPrefix.h>

long sys_mkdir(const char* path, unsigned int mode)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_mkdir, __prefix_translate_path(path), mode);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}

