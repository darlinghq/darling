#include "symlink.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include "../../../../../platform-include/sys/errno.h"

extern char* strcpy(char* dst, const char* src);

long sys_symlink(const char* path, const char* link)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_symlink, path, link);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
