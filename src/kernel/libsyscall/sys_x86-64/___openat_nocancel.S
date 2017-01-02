#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_openat_nocancel
#error "SYS_openat_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___openat_nocancel, openat_nocancel, 4, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_openat$NOCANCEL
	.set	_openat$NOCANCEL, ___openat_nocancel
#endif

