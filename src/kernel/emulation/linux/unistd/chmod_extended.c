#include "chmod_extended.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <libdyld/VirtualPrefix.h>

long sys_chmod_extended(const char* path, int uid, int gid, int mode, void* xsec)
{
	int ret;
	const char* xpath;
	
	xpath = __prefix_translate_path(path);

	ret = LINUX_SYSCALL(__NR_chmod, xpath, mode);
	if (ret < 0)
		return errno_linux_to_bsd(ret);
	
	ret = LINUX_SYSCALL(__NR_chown, xpath, uid, gid);
	if (ret < 0)
		return errno_linux_to_bsd(ret);
	
	return ret;
}

