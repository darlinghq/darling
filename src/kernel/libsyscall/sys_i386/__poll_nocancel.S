#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_poll_nocancel
#error "SYS_poll_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__poll_nocancel, poll_nocancel, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	poll
		poll = __poll_nocancel
	.globl	poll$NOCANCEL$UNIX2003
		poll$NOCANCEL$UNIX2003 = __poll_nocancel
#endif

