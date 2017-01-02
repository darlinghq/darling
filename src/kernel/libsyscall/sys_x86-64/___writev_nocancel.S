#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_writev_nocancel
#error "SYS_writev_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___writev_nocancel, writev_nocancel, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_writev$NOCANCEL
	.set	_writev$NOCANCEL, ___writev_nocancel
#endif

