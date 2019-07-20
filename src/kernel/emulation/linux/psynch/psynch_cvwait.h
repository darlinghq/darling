#ifndef LINUX_PSYNCH_CVWAIT_H
#define LINUX_PSYNCH_CVWAIT_H
#include <stdint.h>

long sys_psynch_cvwait(void* cv, uint64_t cvlsgen, uint32_t cvugen, void * mutex,  uint64_t mugen, uint32_t flags, int64_t sec, uint32_t nsec);

#endif

