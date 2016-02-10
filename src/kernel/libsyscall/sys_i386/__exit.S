#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_exit
#error "SYS_exit not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__exit, exit, 1, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_exit
		_exit = __exit
#endif

