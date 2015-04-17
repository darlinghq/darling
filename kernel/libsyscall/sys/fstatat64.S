#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_fstatat64
#error "SYS_fstatat64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(fstatat64, fstatat64, 4, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	fstatat$INODE64
	.set	fstatat$INODE64, fstatat64
#endif

