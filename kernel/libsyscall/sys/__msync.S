#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_msync
#error "SYS_msync not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__msync, msync, 3, cerror)
#endif

#if defined(__x86_64__)
	.globl	msync
	.set	msync, __msync
#endif

