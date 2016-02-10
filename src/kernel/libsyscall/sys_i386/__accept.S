#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_accept
#error "SYS_accept not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__accept, accept, 3, cerror)
#endif

#if defined(__i386__)
	.globl	accept$UNIX2003
		accept$UNIX2003 = __accept
#endif

