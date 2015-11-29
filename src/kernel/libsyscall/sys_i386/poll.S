#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#if defined(__i386__)
	.globl	poll$UNIX2003
		poll$UNIX2003 = __poll
#endif

