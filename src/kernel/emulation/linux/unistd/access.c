#include "access.h"
#include "../base.h"
#include "../errno.h"
#include "faccessat.h"
#include "../bsdthread/per_thread_wd.h"
#include <linux-syscalls/linux.h>

long sys_access(const char* filename, int amode)
{
	return sys_faccessat(get_perthread_wd(), filename, amode, 0);
}
