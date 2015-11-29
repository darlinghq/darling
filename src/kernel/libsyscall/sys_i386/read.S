#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#if defined(__i386__)
	.globl	read$UNIX2003
		read$UNIX2003 = __read
#endif

