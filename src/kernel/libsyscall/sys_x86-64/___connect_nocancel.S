#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_connect_nocancel
#error "SYS_connect_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___connect_nocancel, connect_nocancel, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_connect$NOCANCEL
	.set	_connect$NOCANCEL, ___connect_nocancel
#endif

