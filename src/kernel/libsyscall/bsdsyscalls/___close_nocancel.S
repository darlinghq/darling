#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_close_nocancel
#error "SYS_close_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___close_nocancel)
SYSCALL_NONAME(close_nocancel, 1, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___close_nocancel, close_nocancel, 1, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_close
	.set	_close, ___close_nocancel
	.globl	_close$NOCANCEL$UNIX2003
	.set	_close$NOCANCEL$UNIX2003, ___close_nocancel
#endif

#if defined(__x86_64__)
	.globl	_close$NOCANCEL
	.set	_close$NOCANCEL, ___close_nocancel
#endif

#if defined(__ppc__)
	.globl	_close
	.set	_close, ___close_nocancel
	.globl	_close$NOCANCEL$UNIX2003
	.set	_close$NOCANCEL$UNIX2003, ___close_nocancel
#endif

#if defined(__arm64__)
	.globl	_close$NOCANCEL
	.set	_close$NOCANCEL, ___close_nocancel
#endif

