#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_setsgroups
#error "SYS_setsgroups not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__setsgroups, setsgroups, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	setsgroups_np
		setsgroups_np = __setsgroups
#endif

