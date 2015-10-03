#include "ipc_msg.h"
#include "ipc_port.h"
#include "debug.h"
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include "task.h"

extern ipc_namespace_t kernel_namespace;

static
mach_msg_return_t ipc_msg_complex_copyin(ipc_namespace_t* space,
							struct ipc_kmsg* kmsg);

static
mach_msg_return_t ipc_process_right(ipc_namespace_t* space,
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
mach_msg_return_t ipc_process_right_abort(ipc_namespace_t* space,
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
mach_msg_return_t ipc_process_right_end(ipc_namespace_t* space,
		mach_msg_type_name_t type,
		mach_port_t in_right_name,
		struct mach_port_right* in_right)
{
	mach_msg_return_t ret = MACH_MSG_SUCCESS;
	
	if (in_right == NULL)
		in_right = ipc_space_lookup(space, in_right_name);
	if (in_right == NULL)
		return ret;
	
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
				ipc_space_name_put(space, in_right_name);
			}
			
			break;
			
		case MACH_MSG_TYPE_MAKE_SEND:
		case MACH_MSG_TYPE_MAKE_SEND_ONCE:
			
			break;
		case MACH_MSG_TYPE_MOVE_SEND_ONCE:
			
			// Destroy the right name in caller
			ipc_right_put(in_right);
			ipc_space_name_put(space, in_right_name);
			
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

mach_msg_return_t ipc_msg_send(ipc_namespace_t* space,
		mach_msg_header_t* msg,
		mach_msg_timeout_t timeout,
		int options)
{
	mach_msg_return_t ret = MACH_MSG_SUCCESS;
	mach_msg_type_name_t type, resp_type;
	struct ipc_kmsg* kmsg;
	
	// The right that we received from caller
	struct mach_port_right* in_remote_right = NULL;
	struct mach_port_right* in_local_right = NULL;
	
	kmsg = (struct ipc_kmsg*) kmalloc(sizeof(struct ipc_kmsg), GFP_KERNEL);
	
	kmsg->reply = NULL;
	kmsg->target = NULL;
	kmsg->msg = msg;
	
	debug_msg("ipc_msg_send(): remote=%d, local=%d\n",
			msg->msgh_remote_port,
			msg->msgh_local_port);
	
	type = MACH_MSGH_BITS_REMOTE(msg->msgh_bits);
	resp_type = MACH_MSGH_BITS_LOCAL(msg->msgh_bits);
	
	ipc_space_lock(space);
	
	// Find the target right/port
	in_remote_right = ipc_space_lookup(space, msg->msgh_remote_port);
	
	if (in_remote_right == NULL)
	{
		ret = MACH_SEND_INVALID_DEST;
		goto err;
	}
	
	// Find the reply right/port
	if (msg->msgh_local_port != 0)
	{
		in_local_right = ipc_space_lookup(space, msg->msgh_local_port);

		if (in_local_right == NULL)
		{
			ret = MACH_SEND_INVALID_REPLY;
			goto err;
		}
	}
	
	// Perform operation specified in type on target right
	ret = ipc_process_right(space, type,
			in_remote_right, &kmsg->target);
	
	if (ret != MACH_MSG_SUCCESS)
		goto err;
	
	if (MACH_MSGH_BITS_IS_COMPLEX(msg->msgh_bits))
	{
		ret = ipc_msg_complex_copyin(space, kmsg);
		if (ret != KERN_SUCCESS)
			goto err;
	}
	
	// Cannot send messages to dead ports
	if (!PORT_IS_VALID(kmsg->target->port))
	{
		ret = MACH_SEND_INVALID_DEST;
		goto err;
	}
	
	// Perform operation specified in type on reply right
	if (in_local_right != NULL)
	{
		ret = ipc_process_right(space, resp_type,
				in_local_right, &kmsg->reply);
		if (ret != MACH_MSG_SUCCESS)
			goto err;
		
		ipc_port_unlock(in_local_right->port);
	}
	
	// Finish operation specified in type on target right
	ipc_process_right_end(space, type, msg->msgh_remote_port, in_remote_right);
	
	// Finish operation specified in type on reply right
	if (in_local_right != NULL)
	{
		ipc_process_right_end(space, resp_type, msg->msgh_local_port,
				in_local_right);
	}
	
	ipc_space_unlock(space);
	
	ret = ipc_msg_deliver(kmsg, timeout, options);
	
	kfree(kmsg);
	return ret;
	
err:

	// Abort operation specified in type on target right
	ipc_process_right_abort(space, type, kmsg->target);
	// Abort operation specified in type on reply right
	ipc_process_right_abort(space, resp_type, kmsg->reply);
	
	kfree(msg);
	
	if (in_remote_right != NULL)
		ipc_port_unlock(in_remote_right->port);
	if (in_local_right != NULL)
		ipc_port_unlock(in_local_right->port);

	kfree(kmsg);
	ipc_space_unlock(space);
	return ret;
}

static
mach_msg_return_t ipc_msg_invoke_server(struct ipc_kmsg* kmsg,
		int options, mach_msg_timeout_t timeout)
{
	mig_subsystem_t subsystem;
	mig_routine_t routine;
	mach_msg_header_t* reply_msg;
	mach_msg_return_t ret;

	subsystem = kmsg->target->port->server_port.subsystem;

	if (kmsg->reply->type != MACH_PORT_RIGHT_SEND_ONCE)
	{
		debug_msg("MIG call, but reply is not send once?\n");
		return KERN_NOT_SUPPORTED;
	}

	if (subsystem == MIG_SUBSYSTEM_NULL)
	{
		debug_msg("ipc_msg_invoke_server(): target port is server,"
				" but no subsystem is assigned\n");
		return KERN_NOT_SUPPORTED;
	}

	routine = subsystem->server(kmsg->msg);
	if (routine == NULL)
	{
		debug_msg("ipc_msg_invoke_server(): invalid routine ID (0x%x)\n",
				kmsg->msg->msgh_id);
		return KERN_INVALID_ARGUMENT;
	}

	reply_msg = (mach_msg_header_t*) kzalloc(sizeof(mach_msg_header_t)
			+ subsystem->maxsize, GFP_KERNEL);
	
	// Insert reply port from kmsg into kernel_namespace
	// and put the right name into reply_msg
	if (kmsg->reply != NULL)
	{
		ipc_space_right_insert(&kernel_namespace, kmsg->reply,
				&reply_msg->msgh_remote_port);
		reply_msg->msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_MOVE_SEND_ONCE, 0);
	}
	else
	{
		reply_msg->msgh_remote_port = 0;
	}

	reply_msg->msgh_id = kmsg->msg->msgh_id + 100;
	// reply_msg->msgh_voucher_port = kmsg->msg->msgh_voucher_port;

	debug_msg("ipc_msg_invoke_server(): invoke routine %d, msg size = %d\n",
			kmsg->msg->msgh_id, kmsg->msg->msgh_size);

	reply_msg->msgh_size = sizeof(mig_reply_error_t);

	routine(kmsg->msg, reply_msg);

	debug_msg("ipc_msg_deliver(): reply size: %d, ret = 0x%x\n",
			reply_msg->msgh_size,
			((mig_reply_error_t*)reply_msg)->RetCode);
	ipc_port_unlock(kmsg->target->port);
	
	// The right used to call us is now consumed.
	ipc_right_put(kmsg->target);
	
	// TODO: prevent circular loops
	ret = ipc_msg_send(&kernel_namespace, reply_msg, timeout, options);
	
	if (ret != MACH_MSG_SUCCESS)
	{
		// TODO: Destroy OOL rights in reply_msg
		ipc_space_right_put(&kernel_namespace, reply_msg->msgh_remote_port);
	}
	
	return ret;
}

/**
 * In: ports are locked.
 * Out: ports are unlocked on success.
 * Destroys passed rights on success.
 */
mach_msg_return_t ipc_msg_deliver(struct ipc_kmsg* kmsg,
		mach_msg_timeout_t timeout,
		int options)
{
	mach_msg_return_t ret = MACH_MSG_SUCCESS;
	struct ipc_delivered_msg* delivery = NULL;
	darling_mach_port_t* port;
	
	debug_msg("ipc_msg_deliver()\n");
	
	// MIG call handling
	if (kmsg->target->port->is_server_port)
	{
		return ipc_msg_invoke_server(kmsg, timeout, options);
	}
	
	port = kmsg->target->port;
	
	// Enqueue message
	delivery = (struct ipc_delivered_msg*) kmalloc(
			sizeof(struct ipc_delivered_msg), GFP_KERNEL);
			
	INIT_LIST_HEAD(&delivery->list);
	memcpy(&delivery->kmsg, kmsg, sizeof(*kmsg));
	delivery->delivered = false;
	delivery->recipient_freed = kmsg->target->type == MACH_PORT_RIGHT_SEND_ONCE;
	
	list_add(&delivery->list, &port->messages);
	port->queue_size++;
	
	// Wake up waiting receivers
	wake_up_interruptible(&port->queue_recv);
	
	// Wait (unless target is send once)
	if (kmsg->target->type != MACH_PORT_RIGHT_SEND_ONCE)
	{	
		int err;
		
		debug_msg("-> waiting for delivery\n");
		
		ipc_port_unlock(port);
		
waiting:
		if (timeout)
		{
			err = wait_event_interruptible_timeout(port->queue_send,
					delivery->delivered, msecs_to_jiffies(timeout));
		}
		else
		{
			err = wait_event_interruptible(port->queue_send,
					delivery->delivered);
		}
		
		if (err) // interruption
		{
			if (options & MACH_SEND_INTERRUPT)
				ret = MACH_SEND_INTERRUPTED;
			else
				goto waiting;
		}

		debug_msg("\t-> kfree(delivery) #1\n");
		kfree(delivery);
	}
	else
	{
		ipc_port_unlock(port);
		delivery = NULL; // Don't free
	}
	
	if (ret != MACH_MSG_SUCCESS)
	{
		// Clean up the queue
		ipc_port_lock(port);
		
		// Port may have died
		if (PORT_IS_VALID(kmsg->target->port) && delivery != NULL)
		{
			if (!delivery->delivered)
			{
				list_del(&delivery->list);
				port->queue_size--;
			}
		}
		
		ipc_port_unlock(port);
	}
	else
	{
		ipc_right_put(kmsg->target);
	}
	
	if (delivery != NULL)
	{
		debug_msg("\t-> kfree(delivery) #2\n");
		kfree(delivery);
	}
	
	return ret;
}

static
bool walk_complex_msg(mach_msg_base_t* base,
		bool (*cb)(mach_msg_type_descriptor_t*,int,void*),
		void* private)
{
	struct complex_msg
	{
		mach_msg_base_t base;
		mach_msg_type_descriptor_t desc; // first descriptor
	};
	
	struct complex_msg* complex;
	char* next_descriptor;
	mach_msg_size_t i;
	
	complex = (struct complex_msg*) base;
	next_descriptor = (char*) &complex->desc;
	
	for (i = 0; i < complex->base.body.msgh_descriptor_count; i++)
	{
		mach_msg_type_descriptor_t* desc;
		
		desc = (mach_msg_type_descriptor_t*) next_descriptor;
		
		switch (desc->type)
		{
			case MACH_MSG_PORT_DESCRIPTOR:
				if (!cb(desc, i, private))
					return false;
				
				next_descriptor += sizeof(mach_msg_port_descriptor_t);
				break;
				
			case MACH_MSG_OOL_DESCRIPTOR:
				if (!cb(desc, i, private))
					return false;
				
				if (task_is_64bit())
					next_descriptor += sizeof(mach_msg_ool_descriptor64_t);
				else
					next_descriptor += sizeof(mach_msg_ool_descriptor32_t);
				break;
			
			case MACH_MSG_OOL_PORTS_DESCRIPTOR:
				if (!cb(desc, i, private))
					return false;
				
				if (task_is_64bit())
					next_descriptor += sizeof(mach_msg_ool_ports_descriptor64_t);
				else
					next_descriptor += sizeof(mach_msg_ool_ports_descriptor32_t);
				break;
		}
	}
	
	return true;
}

struct ipc_msg_copyin_complex_args
{
	ipc_namespace_t* space;
	struct ipc_kmsg* kmsg;
	mach_msg_return_t ret;
};

static
bool __ipc_msg_copyin_complex(mach_msg_type_descriptor_t* desc,
		int index, void* p)
{
	struct ipc_msg_copyin_complex_args* args;
	
	args = (struct ipc_msg_copyin_complex_args*) p;
	
	switch (desc->type)
	{
		case MACH_MSG_PORT_DESCRIPTOR:
		{
			mach_msg_port_descriptor_t* port_desc;
			darling_mach_port_right_t* right;

			port_desc = (mach_msg_port_descriptor_t*) desc;
			
			debug_msg("copyin right %d\n", port_desc->name);

			right = ipc_space_lookup(args->space, port_desc->name);
			if (right == NULL)
			{
				args->ret = MACH_SEND_INVALID_DEST;
				return false;
			}
			args->ret = ipc_process_right(args->space, port_desc->disposition,
					right,
					&args->kmsg->complex_items[index].port);

			ipc_port_unlock(right->port);
			if (args->ret != KERN_SUCCESS)
				return false;

			break;
		}
	}
	return true;
}

static
bool __ipc_msg_copyin_complex_abort(mach_msg_type_descriptor_t* desc,
		int index, void* p)
{
	struct ipc_msg_copyin_complex_args* args;
	
	args = (struct ipc_msg_copyin_complex_args*) p;
	switch (desc->type)
	{
		case MACH_MSG_PORT_DESCRIPTOR:
		{
			mach_msg_port_descriptor_t* port_desc;
			darling_mach_port_t* port;
			
			if (args->kmsg->complex_items[index].port == NULL)
				break;
			
			port_desc = (mach_msg_port_descriptor_t*) desc;
			port = args->kmsg->complex_items[index].port->port;
			
			ipc_process_right_abort(args->space,
					port_desc->disposition,
					args->kmsg->complex_items[index].port);
			
			ipc_port_unlock(port);
			
			break;
		}
	}
	
	return true;
}

static
bool __ipc_msg_copyin_complex_finish(mach_msg_type_descriptor_t* desc,
		int index, void* p)
{
	struct ipc_msg_copyin_complex_args* args;
	
	args = (struct ipc_msg_copyin_complex_args*) p;
	switch (desc->type)
	{
		case MACH_MSG_PORT_DESCRIPTOR:
		{
			mach_msg_port_descriptor_t* port_desc;
			darling_mach_port_t* port;
			
			port_desc = (mach_msg_port_descriptor_t*) desc;
			port = args->kmsg->complex_items[index].port->port;
			
			ipc_process_right_end(args->space,
					port_desc->disposition,
					port_desc->name,
					NULL);
			
			ipc_port_unlock(port);
			
			break;
		}
	}
	
	return true;
}

static
mach_msg_return_t ipc_msg_complex_copyin(ipc_namespace_t* space,
							struct ipc_kmsg* kmsg)
{
	mach_msg_return_t ret = KERN_SUCCESS;
	struct ipc_msg_copyin_complex_args args = {
		.space = space,
		.kmsg = kmsg,
		.ret = KERN_SUCCESS
	};
	struct complex_msg
	{
		mach_msg_base_t base;
		mach_msg_type_descriptor_t desc; // first descriptor
	};
	
	struct complex_msg* complex;
	
	complex = (struct complex_msg*) kmsg->msg;
	
	kmsg->complex_items = (union complex_item*)
			kzalloc(sizeof(void*) * complex->base.body.msgh_descriptor_count,
				GFP_KERNEL);
	
	if (!walk_complex_msg((mach_msg_base_t*) kmsg->msg,
		__ipc_msg_copyin_complex, &args))
	{
		goto err;
	}
	
	walk_complex_msg((mach_msg_base_t*) kmsg->msg,
		__ipc_msg_copyin_complex_finish, &args);
	
	return KERN_SUCCESS;
	
err:
	// Abort all operations
	debug_msg("copying abort, ret=%d\n", ret);

	walk_complex_msg((mach_msg_base_t*) kmsg->msg,
		__ipc_msg_copyin_complex_abort, &args);
	kfree(kmsg->complex_items);
	
	return ret;
}

struct ipc_msg_copyout_complex_args
{
	ipc_namespace_t* space;
	struct ipc_kmsg* kmsg;
	mach_msg_return_t ret;
};

static
bool __ipc_msg_copyout_complex(mach_msg_type_descriptor_t* desc,
		int index, void* p)
{
	struct ipc_msg_copyout_complex_args* args;
	
	args = (struct ipc_msg_copyout_complex_args*) p;
	switch (desc->type)
	{
		case MACH_MSG_PORT_DESCRIPTOR:
		{
			mach_msg_port_descriptor_t* port_desc;
			mach_msg_return_t ret;
			
			port_desc = (mach_msg_port_descriptor_t*) desc;
			port_desc->disposition =
					ipc_right_copyin_type(port_desc->disposition);
			
			ret = ipc_space_right_insert(args->space,
					args->kmsg->complex_items[index].port,
					&port_desc->name);
			if (ret != KERN_SUCCESS)
				args->ret = ret;
			break;
		}
	}
	return true;
}

static
mach_msg_return_t ipc_msg_copyout_complex(ipc_namespace_t* space,
							struct ipc_kmsg* kmsg)
{
	struct ipc_msg_copyout_complex_args args = {
		.space = space,
		.kmsg = kmsg,
		.ret = KERN_SUCCESS
	};
	if (kmsg->complex_items == NULL)
		return KERN_SUCCESS;
	
	walk_complex_msg((mach_msg_base_t*) kmsg->msg,
		__ipc_msg_copyout_complex, &args);
	
	debug_msg("ipc_msg_copyout_complex(): ret=%d\n", args.ret);
	
	kfree(kmsg->complex_items);
	return args.ret;
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
	bool locked;
	
	debug_msg("ipc_msg_recv() on port %d\n",
			port_name);
	
	ipc_space_lock(&task->namespace);
	
	right = ipc_space_lookup(&task->namespace, port_name);
	if (right == NULL || right->type != MACH_PORT_RIGHT_RECEIVE)
	{
		debug_msg("\t-> MACH_RCV_INVALID_NAME\n");
		
		ret = MACH_RCV_INVALID_NAME;
		ipc_space_unlock(&task->namespace);
		goto err;
	}
	
	// Clone the right
	right = ipc_right_new(right->port, MACH_PORT_RIGHT_RECEIVE);
	locked = true;
	
	ipc_space_unlock(&task->namespace);
	
	while (true)
	{
		if (!PORT_IS_VALID(right->port))
		{
			debug_msg("\t-> MACH_RCV_PORT_DIED\n");
			ret = MACH_RCV_PORT_DIED;
			goto err;
		}
		
		// check queue
		if (list_empty(&right->port->messages))
		{
			int err;
			ipc_port_unlock(right->port);
			locked = false;
			
			debug_msg("\t-> going to wait\n");
			
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
					goto err;
				}
				else
					goto waiting;
			}
			
			ipc_port_lock(right->port);
			locked = true;
		}
		else
		{
			// There is a message in queue and we have the lock
			// Dequeue the message and mark it as delivered.
			
			struct ipc_delivered_msg* delivery;
			struct ipc_kmsg* kmsg;
			
			debug_msg("\t-> there's a msg in queue\n");
			delivery = list_first_entry(&right->port->messages,
					struct ipc_delivered_msg, list);
			
			kmsg = &delivery->kmsg;
			right->port->queue_size--;
			
			debug_msg("\t-> msgh_size=%d, receive_limit=%d\n",
					delivery->kmsg.msg->msgh_size, receive_limit);
			if (delivery->kmsg.msg->msgh_size > receive_limit)
			{
				if (!(options & MACH_RCV_LARGE))
				{
					// The message doesn't fit, so it is simply dropped.
					ret = MACH_RCV_TOO_LARGE;
				}
				else
				{
					// TODO: Message size is passed to the recipient,
					// so that he can allocate a larger message buffer.
					// The message is NOT dequeued.
				}
			}
			else
			{
				debug_msg("\t-> copying msg (%d)\n",
						delivery->kmsg.msg->msgh_size);
				
				// Insert reply port into current task's namespace
				if (kmsg->reply != NULL)
				{
					if (ipc_space_right_insert(&task->namespace, kmsg->reply,
							&kmsg->msg->msgh_remote_port) != KERN_SUCCESS)
					{
						ipc_right_put(kmsg->reply);
					}
					else
					{
						// Change the disposition type to recipient style
						mach_msg_type_name_t type;
						
						type = MACH_MSGH_BITS_REMOTE(kmsg->msg->msgh_bits);
						
						kmsg->msg->msgh_bits &= ~MACH_MSGH_BITS_REMOTE_MASK;
						kmsg->msg->msgh_bits |= ipc_right_copyin_type(type);
					}
				}
				
				if (MACH_MSGH_BITS_IS_COMPLEX(delivery->kmsg.msg->msgh_bits))
				{
					// Handle OOL rights, memory regions etc.
					ipc_msg_copyout_complex(&task->namespace,
							&delivery->kmsg);
				}
				
				// Copy over the message to recipient's buffer.
				if (copy_to_user(msg, kmsg->msg, kmsg->msg->msgh_size))
				{
					ret = KERN_INVALID_ADDRESS;
				}
			}
			
			debug_msg("\t-> finalizing\n");
			
			// The message is always recipient-owned.
			kfree(kmsg->msg);
			
			// Dequeue the message
			list_del(&delivery->list);
			
			if (delivery->recipient_freed)
			{
				// The message was delivered through a send-once right.
				// The sender is not waiting for a notification, and we
				// must delete the delivery.
				kfree(delivery);
			}
			else
			{
				// Mark the message as delivered and wake up the sender.
				delivery->delivered = true;
				wake_up_interruptible(&right->port->queue_send);
			}
			
			break;
		}
	}
	
err:
	if (right != NULL)
	{
		if (locked)
			ipc_port_unlock(right->port);
		ipc_right_put(right);
	}
	
	return ret;
}

