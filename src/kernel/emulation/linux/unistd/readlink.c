#include "readlink.h"
#include "../base.h"
#include "../errno.h"
#include "readlinkat.h"
#include "../bsdthread/per_thread_wd.h"
#include <linux-syscalls/linux.h>

extern __SIZE_TYPE__ strlen(const char* str);

long sys_readlink(const char* path, char* buf, int count)
{
	return sys_readlinkat(get_perthread_wd(), path, buf, count);
}
