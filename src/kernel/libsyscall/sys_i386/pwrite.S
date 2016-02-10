#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_pwrite
#error "SYS_pwrite not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__pwrite, pwrite, 1, cerror)
#endif


#if defined(__i386__)
	.globl	pwrite$UNIX2003
		pwrite$UNIX2003 = __pwrite
#endif

