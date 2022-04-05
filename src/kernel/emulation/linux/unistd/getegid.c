#include "getegid.h"
#include "../base.h"
#include <stddef.h>
#include <darlingserver/rpc.h>
#include "../simple.h"
#include "getuid.h"

long sys_getegid(void)
{
	int32_t gid;
	__getuidgid(NULL, &gid);
	return gid;
}

