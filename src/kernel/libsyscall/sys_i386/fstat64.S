#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_fstat64
#error "SYS_fstat64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(fstat64, fstat64, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	fstat$INODE64
		fstat$INODE64 = fstat64
#endif

