#include "chmod.h"
#include "../base.h"
#include "../errno.h"
#include "fchmodat.h"
#include "../bsdthread/per_thread_wd.h"
#include <linux-syscalls/linux.h>

long sys_chmod(const char* path, int mode)
{
	return sys_fchmodat(get_perthread_wd(), path, mode, 0);
}
