#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_fchmod
#error "SYS_fchmod not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__fchmod, fchmod, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	fchmod
		fchmod = __fchmod
#endif

