#include "ipc_space.h"
#include "ipc_port.h"
#include <linux/slab.h>

void ipc_space_init(struct ipc_space_t* space)
{
	mutex_init(&space->mutex);
	idr_init(&space->names);
}

void ipc_space_put(struct ipc_space_t* space)
{
	/* TODO: put all rights */
	idr_destroy(&space->names);
}

mach_msg_return_t ipc_space_make_receive(struct ipc_space_t* space, struct mach_port_t* port, mach_port_name_t* name_out)
{
	mach_msg_return_t ret;
	struct mach_port_right* right;
	int id;
	
	mutex_lock(&space->mutex);
	
	right = (struct mach_port_right*) kmalloc(sizeof(struct mach_port_right), GFP_KERNEL);
	right->port = port;
	right->type = MACH_PORT_RIGHT_RECEIVE;
	
	id = idr_alloc(&space->names, right, 1, -1, GFP_KERNEL);
	if (id < 0)
	{
		ret = KERN_RESOURCE_SHORTAGE;
		goto err;
	}
	
	*name_out = id;
	
	mutex_unlock(&space->mutex);
	
	return KERN_SUCCESS;
err:

	kfree(right);
	mutex_unlock(&space->mutex);
	return ret;
}

mach_msg_return_t ipc_space_make_send(struct ipc_space_t* space, struct mach_port_t* port, bool once, mach_port_name_t* name_out)
{
	mach_msg_return_t ret;
	struct mach_port_right* right;
	int id;
	
	mutex_lock(&space->mutex);
	
	right = (struct mach_port_right*) kmalloc(sizeof(struct mach_port_right), GFP_KERNEL);
	right->port = port;
	right->type = once ? MACH_PORT_RIGHT_SEND_ONCE : MACH_PORT_RIGHT_SEND;
	
	id = idr_alloc(&space->names, right, 1, -1, GFP_KERNEL);
	if (id < 0)
	{
		ret = KERN_RESOURCE_SHORTAGE;
		goto err;
	}
	
	if (once)
		port->num_sorights++;
	else
		port->num_srights++;
	*name_out = id;
	
	mutex_unlock(&space->mutex);
	
	return KERN_SUCCESS;
	
err:

	kfree(right);
	mutex_unlock(&space->mutex);
	return ret;
}

struct mach_port_right* ipc_space_lookup(struct ipc_space_t* space, mach_port_name_t name)
{
	struct mach_port_right* right;
	
	right = (struct mach_port_right*) idr_find(&space->names, name);
	
	if (right != NULL && PORT_IS_VALID(right->port))
		mutex_lock(&right->port->mutex);
	
	return right;
}

mach_msg_return_t ipc_space_right_put(struct ipc_space_t* space, mach_port_name_t name)
{
	mach_msg_return_t ret;
	struct mach_port_right* right;
	
	mutex_lock(&space->mutex);
	
	right = ipc_space_lookup(space, name);
	if (right == NULL)
	{
		mutex_unlock(&space->mutex);
		return KERN_INVALID_NAME;
	}
	
	if (right->type == MACH_PORT_RIGHT_RECEIVE)
	{
		right->port->dead = 1;
		
		if (right->port->num_srights == 0
			&& &right->port->num_sorights == 0)
		{
			mach_port_put(right->port);
			right->port = NULL;
		}
		else
		{
			// TODO: wake up any pending senders
		}
	}
	else if (right->type == MACH_PORT_RIGHT_SEND
		|| right->type == MACH_PORT_RIGHT_SEND_ONCE)
	{
		if (PORT_IS_VALID(right->port))
		{
			if (right->type == MACH_PORT_RIGHT_SEND)
				right->port->num_srights--;
			else if (right->type == MACH_PORT_RIGHT_SEND_ONCE)
				right->port->num_sorights;
		}
	}
	
	idr_remove(&space->names, name);
	kfree(right);
	
	if (right->port != NULL)
		mutex_unlock(&right->port->mutex);
	
	mutex_unlock(&space->mutex);
	return KERN_SUCCESS;
}
