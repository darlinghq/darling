#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_writev_nocancel
#error "SYS_writev_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___writev_nocancel)
SYSCALL_NONAME(writev_nocancel, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___writev_nocancel, writev_nocancel, 3, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_writev
	.set	_writev, ___writev_nocancel
	.globl	_writev$NOCANCEL$UNIX2003
	.set	_writev$NOCANCEL$UNIX2003, ___writev_nocancel
#endif

#if defined(__x86_64__)
	.globl	_writev$NOCANCEL
	.set	_writev$NOCANCEL, ___writev_nocancel
#endif

#if defined(__ppc__)
	.globl	_writev
	.set	_writev, ___writev_nocancel
	.globl	_writev$NOCANCEL$UNIX2003
	.set	_writev$NOCANCEL$UNIX2003, ___writev_nocancel
#endif

#if defined(__arm64__)
	.globl	_writev$NOCANCEL
	.set	_writev$NOCANCEL, ___writev_nocancel
#endif

