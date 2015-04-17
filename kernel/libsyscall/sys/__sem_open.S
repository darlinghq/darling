#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_sem_open
#error "SYS_sem_open not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__sem_open, sem_open, 4, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	sem_open
	.set	sem_open, __sem_open
#endif

