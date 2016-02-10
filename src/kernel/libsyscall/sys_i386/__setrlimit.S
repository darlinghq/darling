#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_setrlimit
#error "SYS_setrlimit not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__setrlimit, setrlimit, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	setrlimit
		setrlimit = __setrlimit
#endif

