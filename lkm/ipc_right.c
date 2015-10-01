#include "mach_includes.h"
#include "ipc_right.h"
#include "ipc_port.h"
#include "ipc_space.h"
#include "debug.h"
#include <linux/slab.h>

struct mach_port_right* ipc_right_new(darling_mach_port_t* port, mach_port_right_t type)
{
	struct mach_port_right* right;
	
	right = (struct mach_port_right*) kmalloc(sizeof(struct mach_port_right), GFP_KERNEL);
	if (right == NULL)
		return NULL;
	
	right->port = port;
	right->type = type;
	right->num_refs = 1;
	
	if (PORT_IS_VALID(port))
	{
		switch (type)
		{
			case MACH_PORT_RIGHT_RECEIVE:
				break;
			case MACH_PORT_RIGHT_SEND:
				port->num_srights++;
				break;
			case MACH_PORT_RIGHT_SEND_ONCE:
				port->num_sorights++;
				break;
		}
		
		list_add(&right->reflist, &port->refs);
	}
	else if (port == PORT_DEAD)
	{
		right->type = MACH_PORT_RIGHT_DEAD_NAME;
	}
	
	debug_msg("Allocated right %p for port %p\n", right, port);
	return right;
}

void ipc_right_put(struct mach_port_right* right)
{
	darling_mach_port_t* port;
	
	debug_msg("ipc_right_put(%p)\n", right);
	if (right == NULL)
		return;
	
	port = right->port;
	
	if (right->type == MACH_PORT_RIGHT_RECEIVE)
	{
		debug_msg("\tCalling ipc_port_put()\n");
		ipc_port_put(port);
	}
	else if (right->type == MACH_PORT_RIGHT_SEND
		|| right->type == MACH_PORT_RIGHT_SEND_ONCE)
	{
		if (PORT_IS_VALID(port))
		{
			debug_msg("\tChanging num of rights\n");
			if (right->type == MACH_PORT_RIGHT_SEND)
				port->num_srights--;
			else if (right->type == MACH_PORT_RIGHT_SEND_ONCE)
				port->num_sorights--;
			
			list_del(&right->reflist);
		}
	}
	
	kfree(right);
	
	debug_msg("Deallocated right %p\n", right);
}

kern_return_t ipc_right_mod_refs(struct mach_port_right* right,
		mach_port_right_t type,
		int refchange)
{
	if (right->type != type)
		return KERN_INVALID_RIGHT;
	
	if (right->num_refs + refchange < 0)
		return KERN_INVALID_VALUE;
	
	if (refchange > 0 && (right->type == MACH_PORT_RIGHT_RECEIVE 
			|| right->type == MACH_PORT_RIGHT_SEND_ONCE || right->type == MACH_PORT_RIGHT_PORT_SET))
	{
		debug_msg("ipc_right_mod_refs() on right %p -> KERN_UREFS_OVERFLOW\n", right);
		return KERN_UREFS_OVERFLOW;
	}
	
	// We deviate from documentation in having only one right type per right name
	right->num_refs += refchange;
	debug_msg("ipc_right_mod_refs() on right=%p, refchange=%d, num_refs=%d\n",
		right, refchange, right->num_refs);
	
	return KERN_SUCCESS;
}

bool ipc_right_put_if_noref(struct mach_port_right* right,
		ipc_namespace_t* space, mach_port_name_t name)
{
	mach_port_right_t type;
	darling_mach_port_t* port;
	
	if (right->num_refs > 0)
		return false;
	
	type = right->type;
	port = right->port;
	
	ipc_right_put(right);
	
	// If the call above did not lead to destruction of the port,
	// we unlock it
	if (type != MACH_PORT_RIGHT_RECEIVE)
		ipc_port_unlock(port);
	
	ipc_space_name_put(space, name);
	return true;
}