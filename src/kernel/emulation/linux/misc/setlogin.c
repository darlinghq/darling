#include "setlogin.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>

long sys_setlogin(const char* buf)
{
	return 0;
}

