#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_semsys
#error "SYS_semsys not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__semsys, semsys, 5, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	semsys
		semsys = __semsys
#endif

