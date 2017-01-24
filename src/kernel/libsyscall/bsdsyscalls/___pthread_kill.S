#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS___pthread_kill
#error "SYS___pthread_kill not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___pthread_kill, __pthread_kill, 2, cerror_nocancel)
#endif

