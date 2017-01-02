#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_msgctl
#error "SYS_msgctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___msgctl, msgctl, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_msgctl
	.set	_msgctl, ___msgctl
#endif

