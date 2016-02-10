#ifndef LINUX_PSYNCH_CVWAIT_H
#define LINUX_PSYNCH_CVWAIT_H
#include <stdint.h>

long sys_psynch_cvwait(void* cv, uint32_t cvgen, uint32_t cvugen, void* mutex, uint32_t mgen,
		uint32_t ugen, uint64_t sec, uint64_t usec);

#endif

