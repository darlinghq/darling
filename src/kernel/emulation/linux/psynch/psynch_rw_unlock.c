#include "psynch_rw_unlock.h"
#include "../base.h"
#include "psynch_errno.h"
#include <linux-syscalls/linux.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"
#include "../simple.h"

long sys_psynch_rw_unlock(void* rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags)
{
	struct psynch_rw_unlock_args args = {
		.rwlock = rwlock,
		.lgenval = lgenval,
		.ugenval = ugenval,
		.rw_wc = rw_wc,
		.flags = flags
	};

	int rv = lkm_call_raw(NR_psynch_rw_unlock, &args);
	return psynch_errno(rv);
}

