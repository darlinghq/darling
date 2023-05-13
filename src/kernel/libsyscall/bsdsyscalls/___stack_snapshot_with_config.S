#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_stack_snapshot_with_config
#error "SYS_stack_snapshot_with_config not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___stack_snapshot_with_config)
ZERO_EXTEND(2)
SYSCALL_NONAME(stack_snapshot_with_config, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___stack_snapshot_with_config, stack_snapshot_with_config, 3, cerror_nocancel)
#endif

#endif

