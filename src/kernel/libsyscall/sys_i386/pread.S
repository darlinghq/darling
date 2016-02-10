#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_pread
#error "SYS_pread not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__pread, pread, 1, cerror)
#endif


#if defined(__i386__)
	.globl	pread$UNIX2003
		pread$UNIX2003 = __pread
#endif

