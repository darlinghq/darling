#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_pwritev_nocancel
#error "SYS_pwritev_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___pwritev_nocancel)
SYSCALL_NONAME(pwritev_nocancel, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___pwritev_nocancel, pwritev_nocancel, 4, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_pwritev
	.set	_pwritev, ___pwritev_nocancel
	.globl	_pwritev$NOCANCEL
	.set	_pwritev$NOCANCEL, ___pwritev_nocancel
#endif

#if defined(__x86_64__)
	.globl	_pwritev$NOCANCEL
	.set	_pwritev$NOCANCEL, ___pwritev_nocancel
#endif

#if defined(__arm64__)
	.globl	_pwritev$NOCANCEL
	.set	_pwritev$NOCANCEL, ___pwritev_nocancel
#endif

