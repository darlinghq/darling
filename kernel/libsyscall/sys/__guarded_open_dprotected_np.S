#define __SYSCALL_32BIT_ARG_BYTES 28
#include "SYS.h"

#ifndef SYS_guarded_open_dprotected_np
#error "SYS_guarded_open_dprotected_np not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__guarded_open_dprotected_np, guarded_open_dprotected_np, 7, cerror_nocancel)
#endif

