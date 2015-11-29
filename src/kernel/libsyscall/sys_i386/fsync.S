#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#if defined(__i386__)
	.globl	fsync$UNIX2003
		fsync$UNIX2003 = __fsync
#endif

