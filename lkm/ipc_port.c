#include "ipc_port.h"
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/list.h>
#include "debug.h"
#include "ipc_right.h"
#include "linuxmach.h"

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
	
	debug_msg("Allocated new port: %p\n", port);
	
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

static
mach_msg_return_t ipc_process_right(mach_task_t* task,
		mach_msg_type_name_t type,
		struct mach_port_right* in_right,
		struct mach_port_right** out_right)
{
	mach_msg_return_t ret = MACH_MSG_SUCCESS;
	
	*out_right = NULL;
	
	switch (type)
	{
		case MACH_MSG_TYPE_COPY_SEND:
		case MACH_MSG_TYPE_MOVE_SEND:
			// in_remote_right must be a send right
			if (in_right->type != MACH_PORT_RIGHT_SEND)
			{
				ret = MACH_SEND_INVALID_TYPE;
				goto err;
			}
			
			*out_right = ipc_right_new(in_right->port, MACH_PORT_RIGHT_SEND);
			if (!*out_right)
			{
				ret = KERN_RESOURCE_SHORTAGE;
				goto err;
			}
			
			break;
			
		case MACH_MSG_TYPE_MAKE_SEND:
		case MACH_MSG_TYPE_MAKE_SEND_ONCE:
			// in_remote_right must be a receive right
			if (in_right->type != MACH_PORT_RIGHT_RECEIVE)
			{
				ret = MACH_SEND_INVALID_TYPE;
				goto err;
			}
			
			*out_right = ipc_right_new(in_right->port,
					(type == MACH_MSG_TYPE_MAKE_SEND) ? MACH_PORT_RIGHT_SEND : MACH_PORT_RIGHT_SEND_ONCE);
			
			break;
		case MACH_MSG_TYPE_MOVE_SEND_ONCE:
			// in_remote_right must be a send-once right
			if (in_right->type != MACH_PORT_RIGHT_SEND_ONCE)
			{
				ret = MACH_SEND_INVALID_TYPE;
				goto err;
			}
			
			*out_right = ipc_right_new(in_right->port, MACH_PORT_RIGHT_SEND_ONCE);
			
			break;
		case MACH_MSG_TYPE_MOVE_RECEIVE:
			// TODO
			ret = MACH_SEND_INVALID_HEADER;
			goto err;
		default:
			ret = MACH_SEND_INVALID_HEADER;
			goto err;
	}
	
	return MACH_MSG_SUCCESS;
	
err:
	if (*out_right != NULL)
		ipc_right_put(*out_right);
	return ret;
}

static
mach_msg_return_t ipc_process_right_abort(mach_task_t* task,
		mach_msg_type_name_t type,
		struct mach_port_right* out_right)
{
	switch (type)
	{
		default:
			if (out_right != NULL)
				ipc_right_put(out_right);
	}
	
	return MACH_MSG_SUCCESS;
}

static
mach_msg_return_t ipc_process_right_end(mach_task_t* task,
		mach_msg_type_name_t type,
		mach_port_t in_right_name,
		struct mach_port_right* in_right)
{
	mach_msg_return_t ret = MACH_MSG_SUCCESS;
	
	switch (type)
	{
		case MACH_MSG_TYPE_COPY_SEND:
			break;
		case MACH_MSG_TYPE_MOVE_SEND:
			
			// Decrement the ref count in sender
			ipc_right_mod_refs(in_right, MACH_PORT_RIGHT_SEND, -1);

			if (in_right->num_refs == 0)
			{
				ipc_right_put(in_right);
				ipc_space_name_put(&task->namespace, in_right_name);
			}
			
			break;
			
		case MACH_MSG_TYPE_MAKE_SEND:
		case MACH_MSG_TYPE_MAKE_SEND_ONCE:
			
			break;
		case MACH_MSG_TYPE_MOVE_SEND_ONCE:
			
			// Destroy the right name in caller
			ipc_right_put(in_right);
			ipc_space_name_put(&task->namespace, in_right_name);
			
			break;
		case MACH_MSG_TYPE_MOVE_RECEIVE:
			// TODO
			ret = MACH_SEND_INVALID_HEADER;
			break;
		default:
			ret = MACH_SEND_INVALID_HEADER;
			break;
	}

	return ret;
}

mach_msg_return_t ipc_msg_send(mach_task_t* task, mach_msg_header_t* msg,
		mach_msg_timeout_t timeout)
{
	mach_msg_return_t ret = MACH_MSG_SUCCESS;
	mach_msg_type_name_t type, resp_type;
	
	// The right that we received from caller
	struct mach_port_right* in_remote_right = NULL;
	// The right which was created from in_remote_right
	struct mach_port_right* out_remote_right = NULL;
	
	struct mach_port_right* in_local_right = NULL;
	struct mach_port_right* out_local_right = NULL;
	
	type = MACH_MSGH_BITS_REMOTE(msg->msgh_bits);
	resp_type = MACH_MSGH_BITS_LOCAL(msg->msgh_bits);
	
	ipc_space_lock(&task->namespace);
	
	// Find the target right/port
	in_remote_right = ipc_space_lookup(&task->namespace, msg->msgh_remote_port);
	
	if (in_remote_right == NULL)
	{
		ret = MACH_SEND_INVALID_DEST;
		goto err;
	}
	
	// Find the reply right/port
	in_local_right = ipc_space_lookup(&task->namespace, msg->msgh_local_port);
	
	if (in_local_right == NULL)
	{
		ret = MACH_SEND_INVALID_REPLY;
		goto err;
	}
	
	// Perform operation specified in type on target right
	ret = ipc_process_right(task, type,
			in_remote_right, &out_remote_right);
	
	if (ret != MACH_MSG_SUCCESS)
		goto err;
	
	// Cannot send messages to dead ports
	if (!PORT_IS_VALID(out_remote_right->port))
	{
		ret = MACH_SEND_INVALID_DEST;
		goto err;
	}
	
	// Perform operation specified in type on reply right
	ret = ipc_process_right(task, resp_type,
			in_local_right, &out_local_right);
	if (ret != MACH_MSG_SUCCESS)
		goto err;
	
	ret = ipc_msg_deliver(msg, out_remote_right, out_local_right, timeout);
	
	// Finish operation specified in type on target right
	ipc_process_right_end(task, type, msg->msgh_remote_port, out_remote_right);
	
	// Finish operation specified in type on reply right
	ipc_process_right_end(task, resp_type, msg->msgh_local_port,
			out_local_right);
	
	if (in_remote_right != NULL)
		ipc_port_unlock(in_remote_right->port);
	if (in_local_right != NULL)
		ipc_port_unlock(in_local_right->port);
	
	ipc_space_unlock(&task->namespace);
	return ret;
	
err:
	// Abort operation specified in type on target right
	ipc_process_right_abort(task, type, out_remote_right);
	// Abort operation specified in type on reply right
	ipc_process_right_abort(task, resp_type, out_local_right);
	
	if (in_remote_right != NULL)
		ipc_port_unlock(in_remote_right->port);
	if (in_local_right != NULL)
		ipc_port_unlock(in_local_right->port);

	ipc_space_unlock(&task->namespace);
	return ret;
}

mach_msg_return_t ipc_msg_deliver(mach_msg_header_t* msg,
		struct mach_port_right* target,
		struct mach_port_right* reply,
		mach_msg_timeout_t timeout)
{
	if (target->port->is_server_port)
	{
		mig_subsystem_t subsystem = target->port->server_port.subsystem;
		if (subsystem == MIG_SUBSYSTEM_NULL)
		{
			return KERN_NOT_SUPPORTED;
		}
	}
	else
	{
		// TODO: Enqueue message and wait
		return KERN_NOT_SUPPORTED;
	}
}
