#define __SYSCALL_32BIT_ARG_BYTES 0
#include "SYS.h"

#ifndef SYS_vfs_purge
#error "SYS_vfs_purge not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_vfs_purge)
SYSCALL_NONAME(vfs_purge, 0, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_vfs_purge, vfs_purge, 0, cerror_nocancel)
#endif

#endif

