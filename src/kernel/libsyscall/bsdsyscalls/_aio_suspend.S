#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_aio_suspend
#error "SYS_aio_suspend not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_aio_suspend)
SYSCALL_NONAME(aio_suspend, 3, cerror)
ret
#else
#if defined(__x86_64__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_aio_suspend, aio_suspend, 3, cerror)
#else
__SYSCALL2(___aio_suspend, aio_suspend, 3, cerror)
#endif

#endif

#if defined(__i386__)
	.globl	_aio_suspend$UNIX2003
	.set	_aio_suspend$UNIX2003, ___aio_suspend
#endif

#if defined(__ppc__)
	.globl	_aio_suspend$UNIX2003
	.set	_aio_suspend$UNIX2003, ___aio_suspend
#endif

