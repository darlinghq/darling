#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_mprotect
#error "SYS_mprotect not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___mprotect)
ZERO_EXTEND(1)
SYSCALL_NONAME(mprotect, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___mprotect, mprotect, 3, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_mprotect$UNIX2003
	.set	_mprotect$UNIX2003, ___mprotect
#endif

#if defined(__x86_64__)
	.globl	_mprotect
	.set	_mprotect, ___mprotect
#endif

#if defined(__ppc__)
	.globl	_mprotect$UNIX2003
	.set	_mprotect$UNIX2003, ___mprotect
#endif

#if defined(__arm__)
	.globl	_mprotect
	.set	_mprotect, ___mprotect
#endif

#if defined(__arm64__)
	.globl	_mprotect
	.set	_mprotect, ___mprotect
#endif

