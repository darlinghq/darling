#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_getsockname
#error "SYS_getsockname not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__getsockname, getsockname, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	getsockname$UNIX2003
		getsockname$UNIX2003 = __getsockname
#endif

