#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_write_nocancel
#error "SYS_write_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___write_nocancel)
ZERO_EXTEND(2)
SYSCALL_NONAME(write_nocancel, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___write_nocancel, write_nocancel, 3, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_write
	.set	_write, ___write_nocancel
	.globl	_write$NOCANCEL$UNIX2003
	.set	_write$NOCANCEL$UNIX2003, ___write_nocancel
#endif

#if defined(__x86_64__)
	.globl	_write$NOCANCEL
	.set	_write$NOCANCEL, ___write_nocancel
#endif

#if defined(__ppc__)
	.globl	_write
	.set	_write, ___write_nocancel
	.globl	_write$NOCANCEL$UNIX2003
	.set	_write$NOCANCEL$UNIX2003, ___write_nocancel
#endif

#if defined(__arm64__)
	.globl	_write$NOCANCEL
	.set	_write$NOCANCEL, ___write_nocancel
#endif

