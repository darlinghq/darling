#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_msgsys
#error "SYS_msgsys not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__msgsys, msgsys, 5, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	msgsys
	.set	msgsys, __msgsys
#endif

