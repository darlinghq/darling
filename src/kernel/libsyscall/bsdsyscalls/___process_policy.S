#define __SYSCALL_32BIT_ARG_BYTES 32
#include "SYS.h"

#ifndef SYS_process_policy
#error "SYS_process_policy not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___process_policy)
SYSCALL_NONAME(process_policy, 7, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___process_policy, process_policy, 7, cerror_nocancel)
#endif

#endif

