#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_umask_extended
#error "SYS_umask_extended not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___umask_extended, umask_extended, 2, cerror_nocancel)
#endif

