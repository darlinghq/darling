#ifndef LINUX_PSYNCH_MUTEXDROP_H
#define LINUX_PSYNCH_MUTEXDROP_H
#include <stdint.h>

long sys_psynch_mutexdrop(void* mutex, uint32_t mgen, uint32_t ugen, uint64_t tid, uint32_t flags);

#endif

