#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#if defined(__i386__)
	.globl	msgrcv$UNIX2003
		msgrcv$UNIX2003 = __msgrcv
#endif

