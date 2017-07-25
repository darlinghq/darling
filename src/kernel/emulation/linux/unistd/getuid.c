#include "getuid.h"
#include "../base.h"
#include <stddef.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"

long sys_getuid(void)
{
	return lkm_call(NR_getuid, NULL);
}

