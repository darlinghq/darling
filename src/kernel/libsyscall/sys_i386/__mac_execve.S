#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS___mac_execve
#error "SYS___mac_execve not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__mac_execve, __mac_execve, 4, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	__sandbox_me
		__sandbox_me = __mac_execve
#endif

