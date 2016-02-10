#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_searchfs
#error "SYS_searchfs not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(searchfs, searchfs, 6, cerror_nocancel)
#endif

