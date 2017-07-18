#include "symlink.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "symlinkat.h"
#include "../bsdthread/per_thread_wd.h"

long sys_symlink(const char* path, const char* link)
{
	return sys_symlinkat(path, get_perthread_wd(), link);
}
