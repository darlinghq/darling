#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#if defined(__i386__)
	.globl	waitid$UNIX2003
		waitid$UNIX2003 = __waitid
#endif

