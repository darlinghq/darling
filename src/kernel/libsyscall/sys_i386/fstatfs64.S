#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_fstatfs64
#error "SYS_fstatfs64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(fstatfs64, fstatfs64, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	fstatfs$INODE64
		fstatfs$INODE64 = fstatfs64
#endif

