#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_semctl
#error "SYS_semctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___semctl, semctl, 4, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_semctl
	.set	_semctl, ___semctl
#endif

