#include "setegid.h"
#include "../base.h"
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"

long sys_setegid(int egid)
{
	struct uidgid ug = {
		.uid = -1,
		.gid = egid
	};

	return lkm_call(NR_setuidgid, &ug);
}

