#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_msgrcv_nocancel
#error "SYS_msgrcv_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__msgrcv_nocancel, msgrcv_nocancel, 5, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	msgrcv$NOCANCEL
		msgrcv$NOCANCEL = __msgrcv_nocancel
#endif

