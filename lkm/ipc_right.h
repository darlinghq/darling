#ifndef IPC_RIGHT_H
#define IPC_RIGHT_H
#include "mach_includes.h"

struct mach_port_right
{
	struct mach_port_t* port;
	mach_port_right_t type;
	
	/* Linked list in ipc_space */
	struct list_head reflist;
};

#define PORT_NULL ((struct mach_port_t*) 0)
#define PORT_DEAD ((struct mach_port_t*) ~0)
#define PORT_IS_VALID(port) (port != PORT_NULL && port != PORT_DEAD)

#endif
