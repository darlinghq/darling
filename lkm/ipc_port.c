#include "ipc_port.h"
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/atomic.h>
#include "debug.h"
#include "darling_task.h"
#include "ipc_right.h"
#include "ipc_space.h"
#include "traps.h"

static atomic_t port_count = ATOMIC_INIT(0);

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
	port->queue_size = 0;
	
	INIT_LIST_HEAD(&port->refs);
	INIT_LIST_HEAD(&port->messages);
	init_waitqueue_head(&port->queue_send);
	init_waitqueue_head(&port->queue_recv);
	
	debug_msg("Allocated new port: %p\n", port);
	
	atomic_inc(&port_count);
	*port_out = port;
	return KERN_SUCCESS;
}

mach_msg_return_t ipc_port_put(darling_mach_port_t* port)
{
	struct list_head* p;
	debug_msg("Deallocating port: %p\n", port);
	
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
		
		debug_msg("Marking right %p as PORT_DEAD\n", ref);
		ref->port = PORT_DEAD;
		ref->type = MACH_PORT_RIGHT_DEAD_NAME;
	}
	debug_msg("All refs to port %p are now dead\n", port);
	
	// TODO: Wake up any pending senders etc.
	
	atomic_dec(&port_count);
	kfree(port);
	return KERN_SUCCESS;
}

void ipc_port_lock(darling_mach_port_t* port)
{
	if (PORT_IS_VALID(port))
		mutex_lock(&port->mutex);
}

void ipc_port_unlock(darling_mach_port_t* port)
{
	if (PORT_IS_VALID(port))
		mutex_unlock(&port->mutex);
}

int ipc_port_count(void)
{
	return atomic_read(&port_count);
}
