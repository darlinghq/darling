#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_sem_wait
#error "SYS_sem_wait not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__sem_wait, sem_wait, 1, cerror)
#endif


#if defined(__i386__)
	.globl	sem_wait$UNIX2003
		sem_wait$UNIX2003 = __sem_wait
#endif

