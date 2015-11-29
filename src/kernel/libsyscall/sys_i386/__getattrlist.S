#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_getattrlist
#error "SYS_getattrlist not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__getattrlist, getattrlist, 5, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	getattrlist$UNIX2003
		getattrlist$UNIX2003 = __getattrlist
#endif

