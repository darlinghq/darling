#ifndef LINUX_PSYNCH_RW_RDLOCK_H
#define LINUX_PSYNCH_RW_RDLOCK_H
#include <stdint.h>

long sys_psynch_rw_rdlock(void* rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);

#endif

