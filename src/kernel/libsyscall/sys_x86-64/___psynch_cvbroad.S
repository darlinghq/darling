#define __SYSCALL_32BIT_ARG_BYTES 44
#include "SYS.h"

#ifndef SYS_psynch_cvbroad
#error "SYS_psynch_cvbroad not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___psynch_cvbroad, psynch_cvbroad, 7, cerror_nocancel)
#endif

