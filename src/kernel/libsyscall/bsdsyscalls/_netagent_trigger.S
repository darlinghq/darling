#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_netagent_trigger
#error "SYS_netagent_trigger not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_netagent_trigger)
ZERO_EXTEND(1)
SYSCALL_NONAME(netagent_trigger, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_netagent_trigger, netagent_trigger, 2, cerror_nocancel)
#endif

#endif

