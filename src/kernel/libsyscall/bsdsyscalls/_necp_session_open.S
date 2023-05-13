#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_necp_session_open
#error "SYS_necp_session_open not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_necp_session_open)
SYSCALL_NONAME(necp_session_open, 1, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_necp_session_open, necp_session_open, 1, cerror_nocancel)
#endif

#endif

