#include "psynch_cvbroad.h"
#include "../base.h"
#include <linux-syscalls/linux.h>
#include <darlingserver/rpc.h>
#include "../simple.h"

long sys_psynch_cvbroad(void* cv, uint64_t cvlsgen, uint64_t cvudgen, uint32_t flags, void* mutex, uint64_t mugen,
		uint64_t tid)
{
	uint32_t retval;
	int ret = dserver_rpc_psynch_cvbroad(cv, cvlsgen, cvudgen, flags, mugen, mugen, tid, &retval);

	if (ret < 0) {
		__simple_printf("psynch_cvbroad failed internally: %d", ret);
		__simple_abort();
	}

	return (ret) ? ret : retval;
}

