#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_psynch_rw_wrlock
#error "SYS_psynch_rw_wrlock not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___psynch_rw_wrlock, psynch_rw_wrlock, 5, cerror_nocancel)
#endif

