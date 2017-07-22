#ifndef LINUX_PSYNCH_CVBROAD_H
#define LINUX_PSYNCH_CVBROAD_H
#include <stdint.h>

long sys_psynch_cvbroad(void* cv, uint64_t cvlsgen, uint64_t cvudgen, uint32_t flags, void* mutex, uint64_t mugen,
		uint64_t tid);

#endif

