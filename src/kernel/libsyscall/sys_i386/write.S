#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#if defined(__i386__)
	.globl	write$UNIX2003
		write$UNIX2003 = __write
#endif

