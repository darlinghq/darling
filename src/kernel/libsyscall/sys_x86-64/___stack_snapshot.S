#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_stack_snapshot
#error "SYS_stack_snapshot not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___stack_snapshot, stack_snapshot, 5, cerror_nocancel)
#endif

