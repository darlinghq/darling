#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_fsgetpath_ext
#error "SYS_fsgetpath_ext not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_fsgetpath_ext)
ZERO_EXTEND(1)
SYSCALL_NONAME(fsgetpath_ext, 5, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_fsgetpath_ext, fsgetpath_ext, 5, cerror_nocancel)
#endif

#endif

