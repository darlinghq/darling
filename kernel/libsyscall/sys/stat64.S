#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_stat64
#error "SYS_stat64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(stat64, stat64, 2, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	stat$INODE64
		stat$INODE64 = stat64
#endif

