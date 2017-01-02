#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_pread_nocancel
#error "SYS_pread_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___pread_nocancel, pread_nocancel, 4, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_pread$NOCANCEL
	.set	_pread$NOCANCEL, ___pread_nocancel
#endif

