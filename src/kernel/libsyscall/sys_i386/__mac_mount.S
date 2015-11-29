#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS___mac_mount
#error "SYS___mac_mount not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__mac_mount, __mac_mount, 5, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	__sandbox_mm
		__sandbox_mm = __mac_mount
#endif

