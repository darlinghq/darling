#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_pwrite_nocancel
#error "SYS_pwrite_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___pwrite_nocancel)
ZERO_EXTEND(2)
SYSCALL_NONAME(pwrite_nocancel, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___pwrite_nocancel, pwrite_nocancel, 4, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_pwrite
	.set	_pwrite, ___pwrite_nocancel
	.globl	_pwrite$NOCANCEL$UNIX2003
	.set	_pwrite$NOCANCEL$UNIX2003, ___pwrite_nocancel
#endif

#if defined(__x86_64__)
	.globl	_pwrite$NOCANCEL
	.set	_pwrite$NOCANCEL, ___pwrite_nocancel
#endif

#if defined(__ppc__)
	.globl	_pwrite
	.set	_pwrite, ___pwrite_nocancel
	.globl	_pwrite$NOCANCEL$UNIX2003
	.set	_pwrite$NOCANCEL$UNIX2003, ___pwrite_nocancel
#endif

#if defined(__arm__)
	.globl	_pwrite$NOCANCEL
	.set	_pwrite$NOCANCEL, ___pwrite_nocancel
#endif

#if defined(__arm64__)
	.globl	_pwrite$NOCANCEL
	.set	_pwrite$NOCANCEL, ___pwrite_nocancel
#endif

