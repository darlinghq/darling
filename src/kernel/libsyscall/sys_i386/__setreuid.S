#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_setreuid
#error "SYS_setreuid not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__setreuid, setreuid, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	setreuid$UNIX2003
		setreuid$UNIX2003 = __setreuid
#endif

