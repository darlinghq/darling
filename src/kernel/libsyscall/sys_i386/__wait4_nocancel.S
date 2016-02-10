#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_wait4_nocancel
#error "SYS_wait4_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__wait4_nocancel, wait4_nocancel, 4, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	wait4
		wait4 = __wait4_nocancel
#endif

