#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_recvfrom
#error "SYS_recvfrom not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___recvfrom, recvfrom, 6, cerror)
#endif

#if defined(__x86_64__)
	.globl	_recvfrom
	.set	_recvfrom, ___recvfrom
#endif

