#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_ioctl
#error "SYS_ioctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___ioctl, ioctl, 3, cerror)
#endif

#if defined(__i386__)
	.globl	_ioctl
	.set	_ioctl, ___ioctl
#endif

