#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_aio_suspend
#error "SYS_aio_suspend not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__aio_suspend, aio_suspend, 1, cerror)
#endif


#if defined(__i386__)
	.globl	aio_suspend$UNIX2003
		aio_suspend$UNIX2003 = __aio_suspend
#endif

