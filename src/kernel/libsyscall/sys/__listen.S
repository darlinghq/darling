#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_listen
#error "SYS_listen not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__listen, listen, 2, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	listen
		listen = __listen
#endif

