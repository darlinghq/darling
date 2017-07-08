#include "unmount.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <sys/errno.h>

long sys_unmount(const char* path, int flags)
{
	return -EBUSY;
}

