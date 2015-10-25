#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_setwgroups
#error "SYS_setwgroups not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__setwgroups, setwgroups, 2, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	setwgroups_np
		setwgroups_np = __setwgroups
#endif

