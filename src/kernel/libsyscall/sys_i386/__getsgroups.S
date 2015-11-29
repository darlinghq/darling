#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_getsgroups
#error "SYS_getsgroups not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__getsgroups, getsgroups, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	getsgroups_np
		getsgroups_np = __getsgroups
#endif

