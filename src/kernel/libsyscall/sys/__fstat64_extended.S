#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_fstat64_extended
#error "SYS_fstat64_extended not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__fstat64_extended, fstat64_extended, 4, cerror_nocancel)
#endif

