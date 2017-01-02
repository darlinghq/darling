#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_ftruncate
#error "SYS_ftruncate not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(_ftruncate, ftruncate, 2, cerror_nocancel)
#endif

