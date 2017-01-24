#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_setreuid
#error "SYS_setreuid not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___setreuid, setreuid, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_setreuid$UNIX2003
	.set	_setreuid$UNIX2003, ___setreuid
#endif

#if defined(__x86_64__)
	.globl	_setreuid
	.set	_setreuid, ___setreuid
#endif

