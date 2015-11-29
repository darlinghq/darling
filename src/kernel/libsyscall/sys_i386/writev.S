#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#if defined(__i386__)
	.globl	writev$UNIX2003
		writev$UNIX2003 = __writev
#endif

