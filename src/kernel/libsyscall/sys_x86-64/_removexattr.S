#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_removexattr
#error "SYS_removexattr not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(_removexattr, removexattr, 3, cerror_nocancel)
#endif

