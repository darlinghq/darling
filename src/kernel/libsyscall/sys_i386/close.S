#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#if defined(__i386__)
	.globl	close$UNIX2003
		close$UNIX2003 = __close
#endif

