#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_read
#error "SYS_read not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__read, read, 1, cerror)
#endif


#if defined(__i386__)
	.globl	read$UNIX2003
		read$UNIX2003 = __read
#endif

