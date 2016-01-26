#include "rename.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include "../../../../../platform-include/sys/errno.h"
#include <libdyld/VirtualPrefix.h>

extern char* strcpy(char* dst, const char* src);

long sys_rename(const char* oldpath, const char* newpath)
{
	int ret;
	char resolved_oldpath[1024];
	char resolved_newpath[1024];
	
	if (!oldpath || !newpath)
		return -EINVAL;

	strcpy(resolved_oldpath, __prefix_translate_path(oldpath));
	strcpy(resolved_newpath, __prefix_translate_path(newpath));

	ret = LINUX_SYSCALL(__NR_rename, resolved_oldpath, resolved_newpath);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}

