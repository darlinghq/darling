#define __SYSCALL_32BIT_ARG_BYTES 32
#include "SYS.h"

#ifndef SYS_process_policy
#error "SYS_process_policy not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__process_policy, process_policy, 7, cerror_nocancel)
#endif

