#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_close_nocancel
#error "SYS_close_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__close_nocancel, close_nocancel, 1, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	close
		close = __close_nocancel
	.globl	close$NOCANCEL$UNIX2003
		close$NOCANCEL$UNIX2003 = __close_nocancel
#endif

