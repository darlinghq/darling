#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_readv_nocancel
#error "SYS_readv_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___readv_nocancel, readv_nocancel, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_readv
	.set	_readv, ___readv_nocancel
	.globl	_readv$NOCANCEL$UNIX2003
	.set	_readv$NOCANCEL$UNIX2003, ___readv_nocancel
#endif

#if defined(__x86_64__)
	.globl	_readv$NOCANCEL
	.set	_readv$NOCANCEL, ___readv_nocancel
#endif

#if defined(__ppc__)
	.globl	_readv
	.set	_readv, ___readv_nocancel
	.globl	_readv$NOCANCEL$UNIX2003
	.set	_readv$NOCANCEL$UNIX2003, ___readv_nocancel
#endif

#if defined(__arm__)
	.globl	_readv$NOCANCEL
	.set	_readv$NOCANCEL, ___readv_nocancel
#endif

#if defined(__arm64__)
	.globl	_readv$NOCANCEL
	.set	_readv$NOCANCEL, ___readv_nocancel
#endif

