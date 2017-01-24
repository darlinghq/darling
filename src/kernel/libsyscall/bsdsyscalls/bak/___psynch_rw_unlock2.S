#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_psynch_rw_unlock2
#error "SYS_psynch_rw_unlock2 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___psynch_rw_unlock2, psynch_rw_unlock2, 5, cerror_nocancel)
#endif

