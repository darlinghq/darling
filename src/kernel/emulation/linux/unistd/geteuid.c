#include "geteuid.h"
#include "../base.h"
#include <stddef.h>
#include <darlingserver/rpc.h>
#include "../simple.h"
#include "getuid.h"

long sys_geteuid(void)
{
	int32_t uid;
	__getuidgid(&uid, NULL);
	return uid;
}

