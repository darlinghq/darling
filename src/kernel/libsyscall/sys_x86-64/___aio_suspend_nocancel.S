#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_aio_suspend_nocancel
#error "SYS_aio_suspend_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___aio_suspend_nocancel, aio_suspend_nocancel, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_aio_suspend$NOCANCEL
	.set	_aio_suspend$NOCANCEL, ___aio_suspend_nocancel
#endif

