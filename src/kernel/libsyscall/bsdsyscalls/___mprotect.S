#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_mprotect
#error "SYS_mprotect not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___mprotect, mprotect, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_mprotect$UNIX2003
	.set	_mprotect$UNIX2003, ___mprotect
#endif

#if defined(__x86_64__)
	.globl	_mprotect
	.set	_mprotect, ___mprotect
#endif

