#include "gettid.h"
#include "../base.h"
#include <stddef.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"

long sys_gettid(int* uid, int* gid)
{
	struct uidgid ug;
	lkm_call(NR_getuidgid, &ug);

	if (uid)
		*uid = ug.uid;

	if (gid)
		*gid = ug.gid;

	return 0;
}

