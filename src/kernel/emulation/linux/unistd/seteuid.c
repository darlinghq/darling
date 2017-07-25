#include "seteuid.h"
#include "../base.h"
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"

long sys_seteuid(int euid)
{
	return lkm_call(NR_setuid, (void*)(long)euid);
}

