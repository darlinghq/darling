#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_posix_spawn
#error "SYS_posix_spawn not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__posix_spawn, posix_spawn, 5, cerror)
#endif

