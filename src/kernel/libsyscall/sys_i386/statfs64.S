#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_statfs64
#error "SYS_statfs64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(statfs64, statfs64, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	statfs$INODE64
		statfs$INODE64 = statfs64
#endif

