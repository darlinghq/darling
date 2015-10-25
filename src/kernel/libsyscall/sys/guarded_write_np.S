#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_guarded_write_np
#error "SYS_guarded_write_np not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(guarded_write_np, guarded_write_np, 4, cerror_nocancel)
#endif

