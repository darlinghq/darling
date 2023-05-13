#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_exit
#error "SYS_exit not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___exit)
SYSCALL_NONAME(exit, 1, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___exit, exit, 1, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	__exit
	.set	__exit, ___exit
#endif

#if defined(__x86_64__)
	.globl	__exit
	.set	__exit, ___exit
#endif

#if defined(__ppc__)
	.globl	__exit
	.set	__exit, ___exit
#endif

#if defined(__arm64__)
	.globl	__exit
	.set	__exit, ___exit
#endif

