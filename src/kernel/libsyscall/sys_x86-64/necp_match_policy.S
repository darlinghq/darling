#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_necp_match_policy
#error "SYS_necp_match_policy not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(necp_match_policy, necp_match_policy, 3, cerror_nocancel)
#endif

