#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_settid
#error "SYS_settid not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__settid, settid, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	pthread_setugid_np
		pthread_setugid_np = __settid
#endif

