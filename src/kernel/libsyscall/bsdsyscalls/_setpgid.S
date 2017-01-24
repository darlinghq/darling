#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_setpgid
#error "SYS_setpgid not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(_setpgid, setpgid, 2, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_setpgrp
	.set	_setpgrp, _setpgid
#endif

