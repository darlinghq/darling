#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_lchown
#error "SYS_lchown not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___lchown, lchown, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_lchown$UNIX2003
	.set	_lchown$UNIX2003, ___lchown
#endif

#if defined(__x86_64__)
	.globl	_lchown
	.set	_lchown, ___lchown
#endif

#if defined(__ppc__)
	.globl	_lchown$UNIX2003
	.set	_lchown$UNIX2003, ___lchown
#endif

#if defined(__arm__)
	.globl	_lchown
	.set	_lchown, ___lchown
#endif

#if defined(__arm64__)
	.globl	_lchown
	.set	_lchown, ___lchown
#endif

