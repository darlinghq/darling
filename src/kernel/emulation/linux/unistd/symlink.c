#include "symlink.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <libdyld/VirtualPrefix.h>
#include "../../../../../platform-include/sys/errno.h"

extern char* strcpy(char* dst, const char* src);

long sys_symlink(const char* path, const char* link)
{
	int ret;
	//char resolved_path[1024];
	char resolved_link[1024];
	
	if (!path || !link)
		return -EINVAL;
	
	//strcpy(resolved_path, __prefix_translate_path(path));
	strcpy(resolved_link, __prefix_translate_path_link(link));

	ret = LINUX_SYSCALL(__NR_symlink, path, resolved_link);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

