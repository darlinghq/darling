#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_waitid_nocancel
#error "SYS_waitid_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___waitid_nocancel)
SYSCALL_NONAME(waitid_nocancel, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___waitid_nocancel, waitid_nocancel, 4, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_waitid
	.set	_waitid, ___waitid_nocancel
	.globl	_waitid$NOCANCEL$UNIX2003
	.set	_waitid$NOCANCEL$UNIX2003, ___waitid_nocancel
#endif

#if defined(__x86_64__)
	.globl	_waitid$NOCANCEL
	.set	_waitid$NOCANCEL, ___waitid_nocancel
#endif

#if defined(__ppc__)
	.globl	_waitid
	.set	_waitid, ___waitid_nocancel
	.globl	_waitid$NOCANCEL$UNIX2003
	.set	_waitid$NOCANCEL$UNIX2003, ___waitid_nocancel
#endif

#if defined(__arm__)
	.globl	_waitid$NOCANCEL
	.set	_waitid$NOCANCEL, ___waitid_nocancel
#endif

#if defined(__arm64__)
	.globl	_waitid$NOCANCEL
	.set	_waitid$NOCANCEL, ___waitid_nocancel
#endif

