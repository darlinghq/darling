#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS___mac_set_link
#error "SYS___mac_set_link not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___mac_set_link, __mac_set_link, 2, cerror_nocancel)
#endif

