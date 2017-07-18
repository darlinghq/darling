#include "mkdir.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "mkdirat.h"
#include "../bsdthread/per_thread_wd.h"

long sys_mkdir(const char* path, unsigned int mode)
{
	return sys_mkdirat(get_perthread_wd(), path, mode);
}
