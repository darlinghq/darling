#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_shm_open
#error "SYS_shm_open not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___shm_open, shm_open, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_shm_open
	.set	_shm_open, ___shm_open
#endif

#if defined(__x86_64__)
	.globl	_shm_open
	.set	_shm_open, ___shm_open
#endif

