#ifndef LINUX_PSYNCH_CVSIGNAL_H
#define LINUX_PSYNCH_CVSIGNAL_H
#include <stdint.h>

long sys_psynch_cvsignal(void* cv, uint32_t cvlsgen, uint32_t cvugen, int thread_port, void* mutex, uint32_t mugen,
		uint64_t tid, uint32_t flags);

#endif

