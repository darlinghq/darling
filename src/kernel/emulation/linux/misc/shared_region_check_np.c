#include "getlogin.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include "../../../../../platform-include/sys/errno.h"

long sys_shared_region_check_np(void* addr)
{
	return -EINVAL; // means: no shared region
}

