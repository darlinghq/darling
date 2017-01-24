#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_fs_snapshot
#error "SYS_fs_snapshot not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___fs_snapshot, fs_snapshot, 6, cerror_nocancel)
#endif

