#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_setsgroups
#error "SYS_setsgroups not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___setsgroups)
SYSCALL_NONAME(setsgroups, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___setsgroups, setsgroups, 2, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_setsgroups_np
	.set	_setsgroups_np, ___setsgroups
#endif

#if defined(__x86_64__)
	.globl	_setsgroups_np
	.set	_setsgroups_np, ___setsgroups
#endif

#if defined(__ppc__)
	.globl	_setsgroups_np
	.set	_setsgroups_np, ___setsgroups
#endif

#if defined(__arm64__)
	.globl	_setsgroups_np
	.set	_setsgroups_np, ___setsgroups
#endif

