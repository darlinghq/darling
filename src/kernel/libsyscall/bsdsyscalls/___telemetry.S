#define __SYSCALL_32BIT_ARG_BYTES 48
#include "SYS.h"

#ifndef SYS_telemetry
#error "SYS_telemetry not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___telemetry)
SYSCALL_NONAME(telemetry, 6, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___telemetry, telemetry, 6, cerror_nocancel)
#endif

#endif

