#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_semctl
#error "SYS_semctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___semctl)
SYSCALL_NONAME(semctl, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___semctl, semctl, 4, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_semctl$UNIX2003
	.set	_semctl$UNIX2003, ___semctl
#endif

#if defined(__x86_64__)
	.globl	_semctl
	.set	_semctl, ___semctl
#endif

#if defined(__ppc__)
	.globl	_semctl$UNIX2003
	.set	_semctl$UNIX2003, ___semctl
#endif

