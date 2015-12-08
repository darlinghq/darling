#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_msgrcv
#error "SYS_msgrcv not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__msgrcv, msgrcv, 1, cerror)
#endif


#if defined(__i386__)
	.globl	msgrcv$UNIX2003
		msgrcv$UNIX2003 = __msgrcv
#endif

