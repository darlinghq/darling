#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_guarded_kqueue_np
#error "SYS_guarded_kqueue_np not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(guarded_kqueue_np, guarded_kqueue_np, 2, cerror_nocancel)
#endif

