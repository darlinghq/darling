#ifndef PRIMITIVES_SEMAPHORE_H
#define PRIMITIVES_SEMAPHORE_H
#include "../ipc_types.h"

void
mach_semaphore_setup(darling_mach_port_t* port, int value);

// All of the following unlock the port on their own

kern_return_t
mach_semaphore_signal(darling_mach_port_t* port);

kern_return_t
mach_semaphore_signal_all(darling_mach_port_t* port);

kern_return_t
mach_semaphore_wait(darling_mach_port_t* port);

kern_return_t
mach_semaphore_timedwait(darling_mach_port_t* port, unsigned int sec,
		unsigned int nsec);

int mach_semaphore_count(void);

#endif
