#include "ipc_port.h"
#include <linux/printk.h>
#include <linux/slab.h>

mach_msg_return_t mach_port_new(struct mach_port_t** port_out)
{
	struct mach_port_t* port;
	
	*port_out = NULL;
	
	port = (struct mach_port_t*) kmalloc(sizeof(struct mach_port_t), GFP_KERNEL);
	if (port == NULL)
		return KERN_RESOURCE_SHORTAGE;
	
	mutex_init(&port->mutex);
	port->dead = 0;
	port->num_srights = 0;
	port->num_sorights = 0;
	
	*port_out = port;
	return KERN_SUCCESS;
}

mach_msg_return_t mach_port_put(struct mach_port_t* port)
{
	kfree(port);
	return KERN_SUCCESS;
}
