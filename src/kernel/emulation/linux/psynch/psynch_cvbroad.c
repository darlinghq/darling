#include "psynch_cvbroad.h"
#include "../base.h"
#include "psynch_errno.h"
#include <linux-syscalls/linux.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"

long sys_psynch_cvbroad(void* cv, uint64_t cvlsgen, uint64_t cvudgen, uint32_t flags, void* mutex, uint64_t mugen,
		uint64_t tid)
{
	struct psynch_cvbroad_args args = {
		.cv = cv,
		.cvlsgen = cvlsgen,
		.cvudgen = cvudgen,
		.mutex = mutex,
		.mugen = mugen,
		.tid = tid,
		.flags = flags,
	};

	int rv = lkm_call(NR_psynch_cvbroad_trap, &args);
	return psynch_errno(rv);
}

