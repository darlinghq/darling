#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_mprotect
#error "SYS_mprotect not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__mprotect, mprotect, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	mprotect
	.set	mprotect, __mprotect
#endif

