#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_getfsstat64
#error "SYS_getfsstat64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(getfsstat64, getfsstat64, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	getfsstat$INODE64
		getfsstat$INODE64 = getfsstat64
#endif

