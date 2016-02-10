#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS___mac_get_lctx
#error "SYS___mac_get_lctx not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__mac_get_lctx, __mac_get_lctx, 1, cerror_nocancel)
#endif

