#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_sem_wait
#error "SYS_sem_wait not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_sem_wait)
SYSCALL_NONAME(sem_wait, 1, cerror)
ret
#else
#if defined(__x86_64__) || defined(__arm64__)
__SYSCALL2(_sem_wait, sem_wait, 1, cerror)
#else
__SYSCALL2(___sem_wait, sem_wait, 1, cerror)
#endif

#endif

#if defined(__i386__)
	.globl	_sem_wait$UNIX2003
	.set	_sem_wait$UNIX2003, ___sem_wait
#endif

#if defined(__ppc__)
	.globl	_sem_wait$UNIX2003
	.set	_sem_wait$UNIX2003, ___sem_wait
#endif

