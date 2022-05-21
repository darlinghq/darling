#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_sendto
#error "SYS_sendto not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___sendto)
ZERO_EXTEND(2)
SYSCALL_NONAME(sendto, 6, cerror)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___sendto, sendto, 6, cerror)
#endif

#endif

#if defined(__i386__)
	.globl	_sendto$UNIX2003
	.set	_sendto$UNIX2003, ___sendto
#endif

#if defined(__x86_64__)
	.globl	_sendto
	.set	_sendto, ___sendto
#endif

#if defined(__ppc__)
	.globl	_sendto$UNIX2003
	.set	_sendto$UNIX2003, ___sendto
#endif

#if defined(__arm64__)
	.globl	_sendto
	.set	_sendto, ___sendto
#endif

