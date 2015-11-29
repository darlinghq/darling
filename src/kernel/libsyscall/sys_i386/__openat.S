#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_openat
#error "SYS_openat not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__openat, openat, 4, cerror)
#endif

#if defined(__i386__)
	.globl	openat
		openat = __openat
#endif

