#include "readlink.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <libdyld/VirtualPrefix.h>

extern __SIZE_TYPE__ strlen(const char* str);

long sys_readlink(const char* path, char* buf, int count)
{
	int ret;

	ret = LINUX_SYSCALL(__NR_readlink, __prefix_translate_path(path),
			buf, count);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);
	else
	{
		const char* xl;
		__SIZE_TYPE__ newlen;
		
		xl = __prefix_untranslate_path(buf, ret);
		newlen = strlen(xl);
		
		memcpy(buf, xl, (newlen < count) ? newlen : count);
		ret = newlen;
	}

	return ret;
}

