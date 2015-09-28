#include "ipc_port.h"
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
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
	port->queue_size = 0;
	
	INIT_LIST_HEAD(&port->refs);
	INIT_LIST_HEAD(&port->messages);
	init_waitqueue_head(&port->queue_send);
	init_waitqueue_head(&port->queue_recv);
	
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

mach_msg_return_t ipc_msg_send(mach_task_t* task,
		mach_msg_header_t* msg,
		mach_msg_timeout_t timeout,
		int options)
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
	
	ipc_space_unlock(&task->namespace);
	
	ret = ipc_msg_deliver(msg, out_remote_right, out_local_right, timeout,
			options);
	
	ipc_space_lock(&task->namespace);
	
	if (ret != MACH_MSG_SUCCESS)
		goto err;
	
	// Finish operation specified in type on target right
	ipc_process_right_end(task, type, msg->msgh_remote_port, in_remote_right);
	
	// Finish operation specified in type on reply right
	ipc_process_right_end(task, resp_type, msg->msgh_local_port,
			in_local_right);
	
	ipc_space_unlock(&task->namespace);
	return ret;
	
err:

	// Abort operation specified in type on target right
	ipc_process_right_abort(task, type, out_remote_right);
	// Abort operation specified in type on reply right
	ipc_process_right_abort(task, resp_type, out_local_right);
	
	kfree(msg);
	
	if (in_remote_right != NULL)
		ipc_port_unlock(in_remote_right->port);
	if (in_local_right != NULL)
		ipc_port_unlock(in_local_right->port);

	ipc_space_unlock(&task->namespace);
	return ret;
}

/**
 * In: ports are locked.
 * Out: ports are unlocked on success.
 * Destroys passed rights on success.
 */
mach_msg_return_t ipc_msg_deliver(mach_msg_header_t* in_msg,
		struct mach_port_right* target,
		struct mach_port_right* reply,
		mach_msg_timeout_t timeout,
		int options)
{	
	mach_msg_return_t ret = MACH_MSG_SUCCESS;
	struct ipc_delivered_msg delivery; // FIXME: must be allocated!
	mach_msg_header_t* msg;
	mach_msg_header_t* reply_msg = NULL;
	struct mach_port_right* orig_target; // Right to destroy on success
	
	// MIG call handling
	if (target->port->is_server_port)
	{
		mig_subsystem_t subsystem;
		mig_routine_t routine;
		
		subsystem = target->port->server_port.subsystem;
		
		if (reply->type != MACH_PORT_RIGHT_SEND_ONCE)
		{
			debug_msg("MIG call, but reply is not send once?\n");
			return KERN_NOT_SUPPORTED;
		}
		
		if (subsystem == MIG_SUBSYSTEM_NULL)
		{
			debug_msg("ipc_msg_deliver(): target port is server,"
					" but no subsystem is assigned\n");
			return KERN_NOT_SUPPORTED;
		}
		
		routine = subsystem->server(in_msg);
		if (routine == NULL)
		{
			debug_msg("ipc_msg_deliver(): invalid routine ID (0x%x)\n",
					in_msg->msgh_id);
			return KERN_INVALID_ARGUMENT;
		}
		
		reply_msg = (mach_msg_header_t*) kmalloc(sizeof(mach_msg_header_t)
				+ subsystem->maxsize, GFP_KERNEL);
		
		reply_msg->msgh_id = in_msg->msgh_id + 100;
		reply_msg->msgh_local_port = MACH_PORT_NULL;
		reply_msg->msgh_remote_port = in_msg->msgh_local_port;
		reply_msg->msgh_bits = in_msg->msgh_bits;
		reply_msg->msgh_voucher_port = in_msg->msgh_voucher_port;
		
		routine(in_msg, reply_msg);
		
		ipc_port_unlock(target->port);
		
		msg = reply_msg;
		orig_target = target;
		target = reply;
	}
	else
	{
		orig_target = NULL;
		msg = in_msg;
	}
	
	// Enqueue message
	INIT_LIST_HEAD(&delivery.list);
	delivery.msg = msg;
	delivery.delivered = false;
	
	if (msg->msgh_local_port != MACH_PORT_NULL)
		delivery.reply = reply;
	else
		delivery.reply = NULL;
	
	list_add(&delivery.list, &target->port->messages);
	
	// Wake up waiting receivers
	wake_up_interruptible(&target->port->queue_recv);
	
	// Wait (unless target is send once)
	if (target->type != MACH_PORT_RIGHT_SEND_ONCE)
	{	
		int err;
		
		ipc_port_unlock(target->port);
		
waiting:
		if (timeout)
		{
			err = wait_event_interruptible_timeout(target->port->queue_send,
					delivery.delivered, msecs_to_jiffies(timeout));
		}
		else
		{
			err = wait_event_interruptible(target->port->queue_send,
					delivery.delivered);
		}
		
		if (err) // interruption
		{
			if (options & MACH_SEND_INTERRUPT)
				ret = MACH_SEND_INTERRUPTED;
			else
				goto waiting;
		}
	}
	else
	{
		ipc_port_unlock(target->port);
	}
	
	if (ret != MACH_MSG_SUCCESS)
	{
		// Clean up the queue
		ipc_port_lock(target->port);
		
		// Port may have died
		if (PORT_IS_VALID(target->port))
		{
			if (!delivery.delivered)
			{
				if (reply_msg != NULL)
					kfree(reply_msg);
				
				list_del(&delivery.list);
			}
		}
	}
	else
	{
		ipc_right_put(target);
		if (orig_target != NULL)
			ipc_right_put(orig_target);
		
		// In case of success, the receiver frees msg.
		// If msg was reply_msg, then we need to free in_msg on our own.
		if (reply_msg != NULL)
			kfree(in_msg);
	}
	
	return ret;
}

mach_msg_return_t ipc_msg_recv(mach_task_t* task,
		mach_port_name_t port_name,
		mach_msg_header_t* msg,
		mach_msg_size_t receive_limit,
		mach_msg_timeout_t timeout,
		int options)
{
	mach_msg_return_t ret = MACH_MSG_SUCCESS;
	struct mach_port_right* right = NULL;
	
	ipc_space_lock(&task->namespace);
	
	right = ipc_space_lookup(&task->namespace, port_name);
	if (right == NULL || right->type != MACH_PORT_RIGHT_RECEIVE)
	{
		ret = MACH_RCV_INVALID_NAME;
		goto err;
	}
	
	while (true)
	{
		if (!PORT_IS_VALID(right->port))
		{
			ret = MACH_RCV_PORT_DIED;
			goto err;
		}
		
		// check queue
		if (list_empty(&right->port->messages))
		{
			int err;
			ipc_port_unlock(right->port);
			
			// wait for ipc_msg_deliver() to be called somewhere
waiting:
			if (timeout)
			{
				err = wait_event_interruptible_timeout(right->port->queue_recv,
						(right->port->queue_size != 0 || !PORT_IS_VALID(right->port)),
						msecs_to_jiffies(timeout));
			}
			else
			{
				err = wait_event_interruptible(right->port->queue_send,
						(right->port->queue_size != 0 || !PORT_IS_VALID(right->port)));
			}
			
			if (ret)
			{
				if (options & MACH_RCV_INTERRUPT)
				{
					ret = MACH_RCV_INTERRUPTED;
					right = NULL; // because we're unlocked
					goto err;
				}
				else
					goto waiting;
			}
			
			ipc_port_lock(right->port);
		}
		else
		{
			// There is a message in queue and we have the lock
			// TODO: Dequeue the message and mark it as delivered
			break;
		}
	}
	
	if (right != NULL)
		ipc_port_unlock(right->port);
	
	ipc_space_unlock(&task->namespace);
	
	return ret;
	
err:
	if (right != NULL)
		ipc_port_unlock(right->port);

	ipc_space_unlock(&task->namespace);
	return ret;
}
