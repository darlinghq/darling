#include "getgid.h"
#include "../base.h"
#include <stddef.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"

long sys_getgid(void)
{
	return lkm_call(NR_getgid, NULL);
}

