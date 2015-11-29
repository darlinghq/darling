#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS___mac_syscall
#error "SYS___mac_syscall not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__mac_syscall, __mac_syscall, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	__sandbox_ms
		__sandbox_ms = __mac_syscall
#endif

