#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_semctl
#error "SYS_semctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__semctl, semctl, 4, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	semctl$UNIX2003
		semctl$UNIX2003 = __semctl
#endif

