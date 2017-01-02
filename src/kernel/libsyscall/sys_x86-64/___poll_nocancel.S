#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_poll_nocancel
#error "SYS_poll_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___poll_nocancel, poll_nocancel, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_poll$NOCANCEL
	.set	_poll$NOCANCEL, ___poll_nocancel
#endif

