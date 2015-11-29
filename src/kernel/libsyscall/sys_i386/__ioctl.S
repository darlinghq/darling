#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_ioctl
#error "SYS_ioctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__ioctl, ioctl, 3, cerror)
#endif

#define ioctl __real_ioctl

#if defined(__i386__)
	.globl	ioctl
		ioctl = __ioctl
#endif

