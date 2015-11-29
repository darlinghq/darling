#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_bind
#error "SYS_bind not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__bind, bind, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	bind$UNIX2003
		bind$UNIX2003 = __bind
#endif

