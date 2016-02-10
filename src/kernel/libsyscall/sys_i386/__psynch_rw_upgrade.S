#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_psynch_rw_upgrade
#error "SYS_psynch_rw_upgrade not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__psynch_rw_upgrade, psynch_rw_upgrade, 5, cerror_nocancel)
#endif

