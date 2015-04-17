#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_poll_nocancel
#error "SYS_poll_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__poll_nocancel, poll_nocancel, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	poll$NOCANCEL
	.set	poll$NOCANCEL, __poll_nocancel
#endif

