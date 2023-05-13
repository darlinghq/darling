#define __SYSCALL_32BIT_ARG_BYTES 40
#include "SYS.h"

#ifndef SYS_proc_info_extended_id
#error "SYS_proc_info_extended_id not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___proc_info_extended_id)
SYSCALL_NONAME(proc_info_extended_id, 8, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___proc_info_extended_id, proc_info_extended_id, 8, cerror_nocancel)
#endif

#endif

