#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_fstat_extended
#error "SYS_fstat_extended not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__fstat_extended, fstat_extended, 4, cerror_nocancel)
#endif

