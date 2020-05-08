#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_necp_match_policy
#error "SYS_necp_match_policy not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_necp_match_policy)
ZERO_EXTEND(1)
SYSCALL_NONAME(necp_match_policy, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_necp_match_policy, necp_match_policy, 3, cerror_nocancel)
#endif

#endif

