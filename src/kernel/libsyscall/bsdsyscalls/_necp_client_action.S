#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_necp_client_action
#error "SYS_necp_client_action not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(_necp_client_action, necp_client_action, 6, cerror_nocancel)
#endif

