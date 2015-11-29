#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#if defined(__i386__)
	.globl	readv$UNIX2003
		readv$UNIX2003 = __readv
#endif

