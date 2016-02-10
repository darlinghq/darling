#include "chdir.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <libdyld/VirtualPrefix.h>

long sys_chdir(const char* in_path)
{
	int ret;
	const char* path;

	path = __prefix_translate_path(in_path);
	ret = LINUX_SYSCALL(__NR_chdir, path);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);
	else
		__prefix_cwd(in_path);

	return ret;
}

