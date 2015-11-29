#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_lstat64
#error "SYS_lstat64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(lstat64, lstat64, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	lstat$INODE64
		lstat$INODE64 = lstat64
#endif

