#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_grab_pgo_data
#error "SYS_grab_pgo_data not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_grab_pgo_data)
SIGN_EXTEND(3)
SYSCALL_NONAME(grab_pgo_data, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_grab_pgo_data, grab_pgo_data, 4, cerror_nocancel)
#endif

#endif

