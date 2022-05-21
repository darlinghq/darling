#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_setrlimit
#error "SYS_setrlimit not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___setrlimit)
SYSCALL_NONAME(setrlimit, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___setrlimit, setrlimit, 2, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_setrlimit
	.set	_setrlimit, ___setrlimit
#endif

#if defined(__ppc__)
	.globl	_setrlimit
	.set	_setrlimit, ___setrlimit
#endif

