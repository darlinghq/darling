#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#if defined(__i386__)
	.globl	pwrite$UNIX2003
		pwrite$UNIX2003 = __pwrite
#endif

