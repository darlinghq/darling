#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#if defined(__i386__)
	.globl	aio_suspend$UNIX2003
		aio_suspend$UNIX2003 = __aio_suspend
#endif

