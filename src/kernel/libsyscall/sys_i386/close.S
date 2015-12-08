#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_close
#error "SYS_close not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__close, close, 1, cerror)
#endif

#if defined(__i386__)
	.globl	close$UNIX2003
		close$UNIX2003 = __close
#endif

