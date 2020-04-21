#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_getwgroups
#error "SYS_getwgroups not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___getwgroups, getwgroups, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_getwgroups_np
	.set	_getwgroups_np, ___getwgroups
#endif

#if defined(__x86_64__)
	.globl	_getwgroups_np
	.set	_getwgroups_np, ___getwgroups
#endif

#if defined(__ppc__)
	.globl	_getwgroups_np
	.set	_getwgroups_np, ___getwgroups
#endif

#if defined(__arm__)
	.globl	_getwgroups_np
	.set	_getwgroups_np, ___getwgroups
#endif

#if defined(__arm64__)
	.globl	_getwgroups_np
	.set	_getwgroups_np, ___getwgroups
#endif

