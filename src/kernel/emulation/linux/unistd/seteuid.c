#include "seteuid.h"
#include "../base.h"
#include "../mach/lkm.h"
#include "../../../../external/lkm/api.h"

long sys_seteuid(int euid)
{
	struct uidgid ug = {
		.uid = euid,
		.gid = -1
	};

	return lkm_call(NR_setuidgid, &ug);
}

