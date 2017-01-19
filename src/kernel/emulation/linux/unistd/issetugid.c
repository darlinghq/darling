#include "issetugid.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "getuid.h"
#include "geteuid.h"
#include "getgid.h"
#include "getegid.h"

long sys_issetugid(void)
{
	return sys_getuid() != sys_geteuid() || sys_getgid() != sys_getegid();
}

