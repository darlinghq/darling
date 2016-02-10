#include "lchown.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <libdyld/VirtualPrefix.h>

long sys_lchown(const char* path, int uid, int gid)
{
	int ret;

#ifdef __NR_chown32
	ret = LINUX_SYSCALL(__NR_lchown32, __prefix_translate_path_link(path), uid, gid);
#else
	ret = LINUX_SYSCALL(__NR_lchown, __prefix_translate_path_link(path), uid, gid);
#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

