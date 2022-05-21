#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_shared_region_map_and_slide_2_np
#error "SYS_shared_region_map_and_slide_2_np not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___shared_region_map_and_slide_2_np)
SYSCALL_NONAME(shared_region_map_and_slide_2_np, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___shared_region_map_and_slide_2_np, shared_region_map_and_slide_2_np, 4, cerror_nocancel)
#endif

#endif

