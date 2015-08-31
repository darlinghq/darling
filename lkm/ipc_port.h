#ifndef IPC_PORT_H
#define IPC_PORT_H
#include "mach_includes.h"
#include <linux/mutex.h>
#include <linux/atomic.h>
#include "ipc_right.h"

struct mach_port_t
{
	struct mutex mutex;
	
	/* TODO: Add list of references */
	

	int num_srights;
	int num_sorights;
};

mach_msg_return_t mach_port_new(struct mach_port_t** port);
mach_msg_return_t mach_port_put(struct mach_port_t* port);

#endif
