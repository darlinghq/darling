#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_shmsys
#error "SYS_shmsys not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___shmsys)
SYSCALL_NONAME(shmsys, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___shmsys, shmsys, 4, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_shmsys
	.set	_shmsys, ___shmsys
#endif

#if defined(__x86_64__)
	.globl	_shmsys
	.set	_shmsys, ___shmsys
#endif

#if defined(__ppc__)
	.globl	_shmsys
	.set	_shmsys, ___shmsys
#endif

