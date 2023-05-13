#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_ioctl
#error "SYS_ioctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___ioctl)
ZERO_EXTEND(1)
SYSCALL_NONAME(ioctl, 3, cerror)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___ioctl, ioctl, 3, cerror)
#endif

#endif

#if defined(__i386__)
	.globl	_ioctl
	.set	_ioctl, ___ioctl
#endif

#if defined(__ppc__)
	.globl	_ioctl
	.set	_ioctl, ___ioctl
#endif

