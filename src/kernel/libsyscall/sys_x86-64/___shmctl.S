#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_shmctl
#error "SYS_shmctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___shmctl, shmctl, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_shmctl
	.set	_shmctl, ___shmctl
#endif

