#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#if defined(__i386__)
	.globl	pread$UNIX2003
		pread$UNIX2003 = __pread
#endif

