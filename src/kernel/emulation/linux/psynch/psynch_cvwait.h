#ifndef LINUX_PSYNCH_CVWAIT_H
#define LINUX_PSYNCH_CVWAIT_H
#include <stdint.h>

long sys_psynch_cvwait(void* cv, uint32_t cvgen, uint32_t cvugen, void* mutex, uint64_t mgen,
		uint32_t ugen, uint64_t sec, uint32_t usec);

#endif

