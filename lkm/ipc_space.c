#include "ipc_space.h"
#include "ipc_port.h"
#include "debug.h"
#include <linux/slab.h>

void ipc_space_init(struct ipc_space_t* space)
{
	debug_msg("ipc_space_init() on space %p\n", space);

	mutex_init(&space->mutex);
	idr_init(&space->names);
}

static int __ipc_right_put(int id, void* p, void* data)
{
	struct mach_port_right* right;
	
	right = (struct mach_port_right*) p;

	ipc_right_put(right);
	
	return 0;
}

void ipc_space_put(struct ipc_space_t* space)
{
	debug_msg("ipc_space_put() on space %p\n", space);

	mutex_lock(&space->mutex);
	
	/* Destroy all rights */
	idr_for_each(&space->names, __ipc_right_put, NULL);
	
	idr_destroy(&space->names);
}

mach_msg_return_t ipc_space_make_receive(struct ipc_space_t* space, darling_mach_port_t* port, mach_port_name_t* name_out)
{
	mach_msg_return_t ret;
	struct mach_port_right* right = NULL;
	int id;
	
	mutex_lock(&space->mutex);
	
	right = ipc_right_new(port, MACH_PORT_RIGHT_RECEIVE);
	
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

	ipc_right_put(right);

	mutex_unlock(&space->mutex);
	return ret;
}

struct idr_right_find_arg
{
	darling_mach_port_t* port;
	mach_port_name_t* name_out;
};

static int __ipc_right_find(int id, void* p, void* data)
{
	struct idr_right_find_arg* arg = (struct idr_right_find_arg*) data;
	struct mach_port_right* right = (struct mach_port_right*) p;
	
	if (right->port == arg->port && right->type == MACH_PORT_RIGHT_SEND)
	{
		right->num_refs++;
		*arg->name_out = id;
		
		/* End the search */
		return 1;
	}
	
	return 0;
}

mach_msg_return_t ipc_space_make_send(struct ipc_space_t* space, darling_mach_port_t* port, bool once, mach_port_name_t* name_out)
{
	mach_msg_return_t ret;
	struct mach_port_right* right = NULL;
	int id;
	
	mutex_lock(&space->mutex);
	
	/* Memory optimization for MACH_PORT_RIGHT_SEND rights */
	if (!once)
	{
		struct idr_right_find_arg arg = { port, name_out };
		
		*name_out = 0;
		
		/* Try to find an existing identical right for this port
		 * and increment its reference count
		 */
		idr_for_each(&space->names, __ipc_right_find, &arg);
		
		if (*name_out != 0)
		{
			mutex_unlock(&space->mutex);
			return KERN_SUCCESS;
		}
	}
	
	right = ipc_right_new(port, once ? MACH_PORT_RIGHT_SEND_ONCE : MACH_PORT_RIGHT_SEND);
	if (right == NULL)
	{
		ret = KERN_RESOURCE_SHORTAGE;
		goto err;
	}
	
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
	ipc_right_put(right);

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
	struct mach_port_right* right;
	darling_mach_port_t* port;
	
	mutex_lock(&space->mutex);
	
	right = ipc_space_lookup(space, name);
	if (right == NULL)
	{
		mutex_unlock(&space->mutex);
		return KERN_INVALID_NAME;
	}
	
	port = right->port;
	
	ipc_space_name_put(space, name);
	ipc_right_put(right);
	
	if (PORT_IS_VALID(port))
		mutex_unlock(&port->mutex);
	
	mutex_unlock(&space->mutex);
	return KERN_SUCCESS;
}

void ipc_space_name_put(struct ipc_space_t* space, mach_port_name_t name)
{
	idr_remove(&space->names, name);
}

void ipc_space_unlock(struct ipc_space_t* space)
{
	mutex_unlock(&space->mutex);
}

void ipc_space_lock(struct ipc_space_t* space)
{
	mutex_lock(&space->mutex);
}
