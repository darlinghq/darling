#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_msync_nocancel
#error "SYS_msync_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__msync_nocancel, msync_nocancel, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	msync$NOCANCEL
	.set	msync$NOCANCEL, __msync_nocancel
#endif

