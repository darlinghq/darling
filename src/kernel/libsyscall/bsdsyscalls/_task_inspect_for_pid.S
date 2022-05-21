#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_task_inspect_for_pid
#error "SYS_task_inspect_for_pid not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_task_inspect_for_pid)
SYSCALL_NONAME(task_inspect_for_pid, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_task_inspect_for_pid, task_inspect_for_pid, 3, cerror_nocancel)
#endif

#endif

