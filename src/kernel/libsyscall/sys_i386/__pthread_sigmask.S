#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS___pthread_sigmask
#error "SYS___pthread_sigmask not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__pthread_sigmask, __pthread_sigmask, 3, cerror_nocancel)
#endif

