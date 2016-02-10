#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_readv
#error "SYS_readv not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__readv, readv, 1, cerror)
#endif


#if defined(__i386__)
	.globl	readv$UNIX2003
		readv$UNIX2003 = __readv
#endif

