#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_guarded_writev_np
#error "SYS_guarded_writev_np not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(_guarded_writev_np, guarded_writev_np, 4, cerror_nocancel)
#endif

