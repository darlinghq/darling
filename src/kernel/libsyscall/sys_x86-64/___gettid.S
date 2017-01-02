#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_gettid
#error "SYS_gettid not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___gettid, gettid, 2, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_pthread_getugid_np
	.set	_pthread_getugid_np, ___gettid
#endif

