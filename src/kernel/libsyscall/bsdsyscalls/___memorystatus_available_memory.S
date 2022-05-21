#define __SYSCALL_32BIT_ARG_BYTES 0
#include "SYS.h"

#ifndef SYS_memorystatus_available_memory
#error "SYS_memorystatus_available_memory not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___memorystatus_available_memory)
SYSCALL_NONAME(memorystatus_available_memory, 0, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___memorystatus_available_memory, memorystatus_available_memory, 0, cerror_nocancel)
#endif

#endif

