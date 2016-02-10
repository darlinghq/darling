#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_waitid
#error "SYS_waitid not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__waitid, waitid, 1, cerror)
#endif


#if defined(__i386__)
	.globl	waitid$UNIX2003
		waitid$UNIX2003 = __waitid
#endif

