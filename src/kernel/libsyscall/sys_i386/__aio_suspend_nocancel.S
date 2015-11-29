#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_aio_suspend_nocancel
#error "SYS_aio_suspend_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__aio_suspend_nocancel, aio_suspend_nocancel, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	aio_suspend
		aio_suspend = __aio_suspend_nocancel
	.globl	aio_suspend$NOCANCEL$UNIX2003
		aio_suspend$NOCANCEL$UNIX2003 = __aio_suspend_nocancel
#endif

