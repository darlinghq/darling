#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_poll
#error "SYS_poll not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__poll, poll, 1, cerror)
#endif


#if defined(__i386__)
	.globl	poll$UNIX2003
		poll$UNIX2003 = __poll
#endif

