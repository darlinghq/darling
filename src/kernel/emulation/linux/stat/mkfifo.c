#include "mkfifo.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <libdyld/VirtualPrefix.h>

#define LINUX_S_IFIFO 0010000

long sys_mkfifo(const char* path, unsigned int mode)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_mknod, __prefix_translate_path(path),
			mode | LINUX_S_IFIFO, 0);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}

