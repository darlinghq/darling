#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_getsockopt
#error "SYS_getsockopt not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(getsockopt, getsockopt, 5, cerror_nocancel)
#endif

