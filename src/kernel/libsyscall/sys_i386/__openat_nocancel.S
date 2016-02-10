#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_openat_nocancel
#error "SYS_openat_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__openat_nocancel, openat_nocancel, 4, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	openat$NOCANCEL
		openat$NOCANCEL = __openat_nocancel
#endif

