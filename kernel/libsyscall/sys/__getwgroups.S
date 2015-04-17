#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_getwgroups
#error "SYS_getwgroups not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__getwgroups, getwgroups, 2, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	getwgroups_np
	.set	getwgroups_np, __getwgroups
#endif

