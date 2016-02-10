#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_msgctl
#error "SYS_msgctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__msgctl, msgctl, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	msgctl$UNIX2003
		msgctl$UNIX2003 = __msgctl
#endif

