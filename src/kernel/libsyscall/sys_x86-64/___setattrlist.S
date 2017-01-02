#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_setattrlist
#error "SYS_setattrlist not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___setattrlist, setattrlist, 5, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_setattrlist
	.set	_setattrlist, ___setattrlist
#endif

