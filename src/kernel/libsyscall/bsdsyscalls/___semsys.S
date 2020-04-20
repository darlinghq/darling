#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_semsys
#error "SYS_semsys not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___semsys, semsys, 5, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_semsys
	.set	_semsys, ___semsys
#endif

#if defined(__x86_64__)
	.globl	_semsys
	.set	_semsys, ___semsys
#endif

#if defined(__ppc__)
	.globl	_semsys
	.set	_semsys, ___semsys
#endif

#if defined(__arm__)
	.globl	_semsys
	.set	_semsys, ___semsys
#endif

