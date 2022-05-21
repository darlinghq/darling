#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_shmctl
#error "SYS_shmctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___shmctl)
SYSCALL_NONAME(shmctl, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___shmctl, shmctl, 3, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_shmctl$UNIX2003
	.set	_shmctl$UNIX2003, ___shmctl
#endif

#if defined(__x86_64__)
	.globl	_shmctl
	.set	_shmctl, ___shmctl
#endif

#if defined(__ppc__)
	.globl	_shmctl$UNIX2003
	.set	_shmctl$UNIX2003, ___shmctl
#endif

#if defined(__arm64__)
	.globl	_shmctl
	.set	_shmctl, ___shmctl
#endif

