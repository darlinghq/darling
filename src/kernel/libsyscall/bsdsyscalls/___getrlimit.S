#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_getrlimit
#error "SYS_getrlimit not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___getrlimit, getrlimit, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_getrlimit
	.set	_getrlimit, ___getrlimit
#endif

#if defined(__ppc__)
	.globl	_getrlimit
	.set	_getrlimit, ___getrlimit
#endif

