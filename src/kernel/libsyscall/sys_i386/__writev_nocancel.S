#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_writev_nocancel
#error "SYS_writev_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__writev_nocancel, writev_nocancel, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	writev
		writev = __writev_nocancel
	.globl	writev$NOCANCEL$UNIX2003
		writev$NOCANCEL$UNIX2003 = __writev_nocancel
#endif

