#include "psynch_cvsignal.h"
#include "../base.h"
#include "psynch_errno.h"
#include <linux-syscalls/linux.h>
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"
#include "../simple.h"

long sys_psynch_cvsignal(void* cv, uint32_t cvlsgen, uint32_t cvugen, int thread_port, void* mutex, uint32_t mugen,
		uint64_t tid, uint32_t flags)
{
	struct psynch_cvsignal_args args = {
		.cv = cv,
		.cvlsgen = cvlsgen,
		.cvugen = cvugen,
		.mutex = mutex,
		.mugen = mugen,
		.tid = tid,
		.thread_port = thread_port,
		.flags = flags,
	};

	// __simple_printf("sys_psynch_mutexwait(mutex=%p, mgen=%x)\n", mutex, mgen);

	int rv = lkm_call(NR_psynch_cvsignal_trap, &args);
	return psynch_errno(rv);
}

