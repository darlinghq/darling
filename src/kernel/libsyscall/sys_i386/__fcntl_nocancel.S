#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_fcntl_nocancel
#error "SYS_fcntl_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__fcntl_nocancel, fcntl_nocancel, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	fcntl
		fcntl = __fcntl_nocancel
	.globl	fcntl$NOCANCEL$UNIX2003
		fcntl$NOCANCEL$UNIX2003 = __fcntl_nocancel
#endif

