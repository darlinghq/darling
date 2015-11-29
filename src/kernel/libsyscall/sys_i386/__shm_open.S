#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_shm_open
#error "SYS_shm_open not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__shm_open, shm_open, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	shm_open
		shm_open = __shm_open
#endif

