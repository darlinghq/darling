#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_sem_wait_nocancel
#error "SYS_sem_wait_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__sem_wait_nocancel, sem_wait_nocancel, 1, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	sem_wait
		sem_wait = __sem_wait_nocancel
	.globl	sem_wait$NOCANCEL$UNIX2003
		sem_wait$NOCANCEL$UNIX2003 = __sem_wait_nocancel
#endif

