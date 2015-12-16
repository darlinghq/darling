#ifndef LINUX_PSYNCH_CVBROAD_H
#define LINUX_PSYNCH_CVBROAD_H
#include <stdint.h>

long sys_psynch_cvbroad(void* cv, uint32_t cvgen, uint32_t diffgen, void* mutex, uint32_t mgen,
		uint32_t ugen, uint64_t tid, uint32_t flags);

#endif

