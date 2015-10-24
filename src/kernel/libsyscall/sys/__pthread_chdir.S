#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS___pthread_chdir
#error "SYS___pthread_chdir not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__pthread_chdir, __pthread_chdir, 1, cerror_nocancel)
#endif

