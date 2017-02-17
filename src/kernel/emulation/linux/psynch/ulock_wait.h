#ifndef LINUX_ULOCK_WAIT_H
#define LINUX_ULOCK_WAIT_H
#include <stdint.h>

long sys_ulock_wait(uint32_t operation, void* addr, uint64_t value, uint32_t timeout);

#define UL_OPCODE_MASK		0xff
#define UL_COMPARE_AND_WAIT	1
#define UL_UNFAIR_LOCK		2
#define ULF_NO_ERRNO		0x1000000

#define FUTEX_WAIT			0
#define FUTEX_PRIVATE_FLAG	128
#define FUTEX_LOCK_PI		6

#endif

