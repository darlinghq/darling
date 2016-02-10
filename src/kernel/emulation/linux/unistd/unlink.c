#include "unlink.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <libdyld/VirtualPrefix.h>

long sys_unlink(const char* path)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_unlink, __prefix_translate_path_link(path));
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

