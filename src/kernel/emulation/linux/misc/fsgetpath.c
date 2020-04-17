#include "fsgetpath.h"
#include <stdint.h>
#include "../duct_errno.h"
#include "../../../../../platform-include/sys/errno.h"

long sys_fsgetpath(const char* buf, unsigned long size, void* fsid, unsigned long long objid)
{
	return -ENOTSUP;
}
