#ifndef LINUX_PSYNCH_CVSIGNAL_H
#define LINUX_PSYNCH_CVSIGNAL_H
#include <stdint.h>

long sys_psynch_cvsignal(void* cv, uint32_t cvgen, uint32_t cvugen, void* mutex, uint32_t mgen,
		uint32_t ugen, int thread_port, uint32_t flags);

#endif

