#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_msgsnd
#error "SYS_msgsnd not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__msgsnd, msgsnd, 1, cerror)
#endif


#if defined(__i386__)
	.globl	msgsnd$UNIX2003
		msgsnd$UNIX2003 = __msgsnd
#endif

