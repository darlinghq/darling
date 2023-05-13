#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_fchmod
#error "SYS_fchmod not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___fchmod)
SYSCALL_NONAME(fchmod, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___fchmod, fchmod, 2, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_fchmod
	.set	_fchmod, ___fchmod
#endif

#if defined(__ppc__)
	.globl	_fchmod
	.set	_fchmod, ___fchmod
#endif

