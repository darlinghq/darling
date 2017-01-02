#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_readv_nocancel
#error "SYS_readv_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___readv_nocancel, readv_nocancel, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_readv$NOCANCEL
	.set	_readv$NOCANCEL, ___readv_nocancel
#endif

