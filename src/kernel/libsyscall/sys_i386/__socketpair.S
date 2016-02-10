#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_socketpair
#error "SYS_socketpair not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__socketpair, socketpair, 4, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	socketpair$UNIX2003
		socketpair$UNIX2003 = __socketpair
#endif

