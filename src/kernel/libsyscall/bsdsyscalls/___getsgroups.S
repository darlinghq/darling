#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_getsgroups
#error "SYS_getsgroups not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___getsgroups, getsgroups, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_getsgroups_np
	.set	_getsgroups_np, ___getsgroups
#endif

#if defined(__x86_64__)
	.globl	_getsgroups_np
	.set	_getsgroups_np, ___getsgroups
#endif

