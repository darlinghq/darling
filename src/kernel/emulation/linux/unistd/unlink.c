#include "unlink.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "unlinkat.h"
#include "../bsdthread/per_thread_wd.h"

long sys_unlink(const char* path)
{
	return sys_unlinkat(get_perthread_wd(), path, 0);
}
