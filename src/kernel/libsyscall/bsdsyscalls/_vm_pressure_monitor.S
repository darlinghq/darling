#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_vm_pressure_monitor
#error "SYS_vm_pressure_monitor not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_vm_pressure_monitor)
SYSCALL_NONAME(vm_pressure_monitor, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_vm_pressure_monitor, vm_pressure_monitor, 3, cerror_nocancel)
#endif

#endif

