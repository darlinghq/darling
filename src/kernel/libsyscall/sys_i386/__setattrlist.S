#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_setattrlist
#error "SYS_setattrlist not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__setattrlist, setattrlist, 5, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	setattrlist$UNIX2003
		setattrlist$UNIX2003 = __setattrlist
#endif

