#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS___mac_set_proc
#error "SYS___mac_set_proc not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__mac_set_proc, __mac_set_proc, 1, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	__sandbox_msp
		__sandbox_msp = __mac_set_proc
#endif

