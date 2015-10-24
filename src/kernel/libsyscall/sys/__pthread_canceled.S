#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS___pthread_canceled
#error "SYS___pthread_canceled not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__pthread_canceled, __pthread_canceled, 1, cerror_nocancel)
#endif

