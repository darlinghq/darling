#define __SYSCALL_32BIT_ARG_BYTES 28
#include "SYS.h"

#ifndef SYS_mmap
#error "SYS_mmap not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___mmap, mmap, 6, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_mmap
	.set	_mmap, ___mmap
#endif

#if defined(__ppc__)
	.globl	_mmap
	.set	_mmap, ___mmap
#endif

