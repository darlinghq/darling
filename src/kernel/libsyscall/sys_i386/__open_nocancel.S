#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_open_nocancel
#error "SYS_open_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__open_nocancel, open_nocancel, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	open$NOCANCEL$UNIX2003
		open$NOCANCEL$UNIX2003 = __open_nocancel
#endif

