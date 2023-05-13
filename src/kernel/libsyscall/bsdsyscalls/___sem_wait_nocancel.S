#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_sem_wait_nocancel
#error "SYS_sem_wait_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___sem_wait_nocancel)
SYSCALL_NONAME(sem_wait_nocancel, 1, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___sem_wait_nocancel, sem_wait_nocancel, 1, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_sem_wait
	.set	_sem_wait, ___sem_wait_nocancel
	.globl	_sem_wait$NOCANCEL$UNIX2003
	.set	_sem_wait$NOCANCEL$UNIX2003, ___sem_wait_nocancel
#endif

#if defined(__x86_64__)
	.globl	_sem_wait$NOCANCEL
	.set	_sem_wait$NOCANCEL, ___sem_wait_nocancel
#endif

#if defined(__ppc__)
	.globl	_sem_wait
	.set	_sem_wait, ___sem_wait_nocancel
	.globl	_sem_wait$NOCANCEL$UNIX2003
	.set	_sem_wait$NOCANCEL$UNIX2003, ___sem_wait_nocancel
#endif

#if defined(__arm64__)
	.globl	_sem_wait$NOCANCEL
	.set	_sem_wait$NOCANCEL, ___sem_wait_nocancel
#endif

