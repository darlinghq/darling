#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS___disable_threadsignal
#error "SYS___disable_threadsignal not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__disable_threadsignal, __disable_threadsignal, 1, cerror_nocancel)
#endif

