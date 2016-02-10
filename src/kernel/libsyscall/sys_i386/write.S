#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_write
#error "SYS_write not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__write, write, 1, cerror)
#endif


#if defined(__i386__)
	.globl	write$UNIX2003
		write$UNIX2003 = __write
#endif

