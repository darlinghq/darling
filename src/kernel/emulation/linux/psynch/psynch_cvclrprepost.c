#include "psynch_cvclrprepost.h"
#include "../base.h"
#include "psynch_errno.h"
#include <linux-syscalls/linux.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"
#include "../simple.h"

long sys_psynch_cvclrprepost(void* cv, uint32_t cvgen, uint32_t cvugen, uint32_t cvsgen, uint32_t prepocnt, uint32_t preposeq, uint32_t flags)
{
	struct psynch_cvclrprepost_args args = {
		.cv = cv,
		.cvgen = cvgen,
		.cvugen = cvugen,
		.cvsgen = cvsgen,
		.prepocnt = prepocnt,
		.preposeq = preposeq,
		.flags = flags
	};

	int rv = lkm_call(NR_psynch_cvclrprepost, &args);
	return psynch_errno(rv);
}

