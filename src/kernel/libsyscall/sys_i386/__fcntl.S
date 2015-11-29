#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_fcntl
#error "SYS_fcntl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__fcntl, fcntl, 3, cerror)
#endif

#if defined(__i386__)
	.globl	fcntl$UNIX2003
		fcntl$UNIX2003 = __fcntl
#endif

