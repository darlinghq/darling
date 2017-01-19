#include "pathconf.h"
#include "fpathconf.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

long sys_pathconf(const char* file, int name)
{
	return sys_fpathconf(-1, name);
}

