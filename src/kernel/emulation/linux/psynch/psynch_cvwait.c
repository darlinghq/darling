#include "psynch_cvwait.h"
#include "../base.h"
#include "psynch_errno.h"
#include <linux-syscalls/linux.h>
#include "../mach/lkm.h"
#include "../../../../external/lkm/api.h"
#include "../simple.h"


long sys_psynch_cvwait(void* cv, uint64_t cvlsgen, uint32_t cvugen, void * mutex, uint64_t mugen, 
		uint32_t flags, int64_t sec, uint32_t nsec)
{
	struct psynch_cvwait_args args = {
		.cv = cv,
		.cvlsgen = cvlsgen,
		.cvugen = cvugen,
		.mutex = mutex,
		.mugen = mugen,
		.flags = flags,
		.sec = sec,
		.nsec = nsec,
	};

	// __simple_printf("sys_psynch_mutexwait(mutex=%p, mgen=%x)\n", mutex, mgen);

	int rv = lkm_call_raw(NR_psynch_cvwait_trap, &args);
	return psynch_errno(rv);
}

