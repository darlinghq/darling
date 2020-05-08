#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_setregid
#error "SYS_setregid not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___setregid)
SYSCALL_NONAME(setregid, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___setregid, setregid, 2, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_setregid$UNIX2003
	.set	_setregid$UNIX2003, ___setregid
#endif

#if defined(__x86_64__)
	.globl	_setregid
	.set	_setregid, ___setregid
#endif

#if defined(__ppc__)
	.globl	_setregid$UNIX2003
	.set	_setregid$UNIX2003, ___setregid
#endif

#if defined(__arm__)
	.globl	_setregid
	.set	_setregid, ___setregid
#endif

#if defined(__arm64__)
	.globl	_setregid
	.set	_setregid, ___setregid
#endif

