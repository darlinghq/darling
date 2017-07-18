#include "rename.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "renameat.h"
#include "../bsdthread/per_thread_wd.h"

long sys_rename(const char* oldpath, const char* newpath)
{
	int fd = get_perthread_wd();
	return sys_renameat(fd, oldpath, fd, newpath);
}
