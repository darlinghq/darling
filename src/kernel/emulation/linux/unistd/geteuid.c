#include "geteuid.h"
#include "../base.h"
#include <stddef.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"

long sys_geteuid(void)
{
	return lkm_call(NR_getuid, NULL);
}

