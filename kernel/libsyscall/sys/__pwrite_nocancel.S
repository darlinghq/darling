#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_pwrite_nocancel
#error "SYS_pwrite_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__pwrite_nocancel, pwrite_nocancel, 4, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	pwrite$NOCANCEL
	.set	pwrite$NOCANCEL, __pwrite_nocancel
#endif

