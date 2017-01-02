#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_guarded_pwrite_np
#error "SYS_guarded_pwrite_np not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(_guarded_pwrite_np, guarded_pwrite_np, 5, cerror_nocancel)
#endif

