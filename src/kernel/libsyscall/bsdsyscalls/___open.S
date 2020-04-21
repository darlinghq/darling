#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_open
#error "SYS_open not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___open, open, 3, cerror)
#endif

#if defined(__i386__)
	.globl	_open$UNIX2003
	.set	_open$UNIX2003, ___open
#endif

#if defined(__x86_64__)
	.globl	_open
	.set	_open, ___open
#endif

#if defined(__ppc__)
	.globl	_open$UNIX2003
	.set	_open$UNIX2003, ___open
#endif

#if defined(__arm__)
	.globl	_open
	.set	_open, ___open
#endif

