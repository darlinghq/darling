#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#if defined(__i386__)
	.globl	msgsnd$UNIX2003
		msgsnd$UNIX2003 = __msgsnd
#endif

