#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_writev
#error "SYS_writev not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__writev, writev, 1, cerror)
#endif


#if defined(__i386__)
	.globl	writev$UNIX2003
		writev$UNIX2003 = __writev
#endif

