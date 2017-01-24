#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_openat
#error "SYS_openat not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___openat, openat, 4, cerror)
#endif

#if defined(__i386__)
	.globl	_openat
	.set	_openat, ___openat
#endif

#if defined(__x86_64__)
	.globl	_openat
	.set	_openat, ___openat
#endif

