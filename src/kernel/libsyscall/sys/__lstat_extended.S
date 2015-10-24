#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_lstat_extended
#error "SYS_lstat_extended not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__lstat_extended, lstat_extended, 4, cerror_nocancel)
#endif

