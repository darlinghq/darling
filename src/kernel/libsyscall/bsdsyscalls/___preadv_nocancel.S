#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_preadv_nocancel
#error "SYS_preadv_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___preadv_nocancel)
SYSCALL_NONAME(preadv_nocancel, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___preadv_nocancel, preadv_nocancel, 4, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_preadv
	.set	_preadv, ___preadv_nocancel
	.globl	_preadv$NOCANCEL
	.set	_preadv$NOCANCEL, ___preadv_nocancel
#endif

#if defined(__x86_64__)
	.globl	_preadv$NOCANCEL
	.set	_preadv$NOCANCEL, ___preadv_nocancel
#endif

#if defined(__arm64__)
	.globl	_preadv$NOCANCEL
	.set	_preadv$NOCANCEL, ___preadv_nocancel
#endif

