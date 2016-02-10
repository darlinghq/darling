#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_write_nocancel
#error "SYS_write_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__write_nocancel, write_nocancel, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	write
		write = __write_nocancel
	.globl	write$NOCANCEL$UNIX2003
		write$NOCANCEL$UNIX2003 = __write_nocancel
#endif

