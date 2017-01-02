#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_write_nocancel
#error "SYS_write_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___write_nocancel, write_nocancel, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_write$NOCANCEL
	.set	_write$NOCANCEL, ___write_nocancel
#endif

