#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_sendto
#error "SYS_sendto not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___sendto, sendto, 6, cerror)
#endif

#if defined(__x86_64__)
	.globl	_sendto
	.set	_sendto, ___sendto
#endif

