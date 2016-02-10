#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_readv_nocancel
#error "SYS_readv_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__readv_nocancel, readv_nocancel, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	readv
		readv = __readv_nocancel
	.globl	readv$NOCANCEL$UNIX2003
		readv$NOCANCEL$UNIX2003 = __readv_nocancel
#endif

