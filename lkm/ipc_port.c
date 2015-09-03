#include "ipc_port.h"
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/list.h>
#include "debug.h"
#include "ipc_right.h"

mach_msg_return_t ipc_port_new(darling_mach_port_t** port_out)
{
	darling_mach_port_t* port;
	
	*port_out = NULL;
	
	port = (darling_mach_port_t*) kmalloc(sizeof(darling_mach_port_t), GFP_KERNEL);
	if (port == NULL)
		return KERN_RESOURCE_SHORTAGE;
	
	mutex_init(&port->mutex);
	port->num_srights = 0;
	port->num_sorights = 0;
	port->is_server_port = false;
	
	INIT_LIST_HEAD(&port->refs);
	
	debug_msg("Allocated new port: %p", port);
	
	*port_out = port;
	return KERN_SUCCESS;
}

mach_msg_return_t ipc_port_put(darling_mach_port_t* port)
{
	struct list_head* p;
	debug_msg("Deallocating port: %p", port);
	
	if (port->is_server_port)
	{
		if (port->server_port.cb_free != NULL)
			port->server_port.cb_free(&port->server_port);
	}
	
	// mark all references as dead ports
	list_for_each(p, &port->refs)
	{
		struct mach_port_right* ref;

		ref = list_entry(p, struct mach_port_right, reflist);
		
		debug_msg("Marking right %p as PORT_DEAD", ref);
		ref->port = PORT_DEAD;
		ref->type = MACH_PORT_RIGHT_DEAD_NAME;
	}
	
	// TODO: Wake up any pending senders etc.
	
	kfree(port);
	return KERN_SUCCESS;
}

void ipc_port_unlock(darling_mach_port_t* port)
{
	if (PORT_IS_VALID(port))
		mutex_unlock(&port->mutex);
}
