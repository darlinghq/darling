#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_chmod
#error "SYS_chmod not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___chmod)
SYSCALL_NONAME(chmod, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___chmod, chmod, 2, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_chmod
	.set	_chmod, ___chmod
#endif

#if defined(__ppc__)
	.globl	_chmod
	.set	_chmod, ___chmod
#endif

