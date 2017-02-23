/*
 * Darling Mach Linux Kernel Module
 * Copyright (C) 2015 Lubos Dolezel
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "mach_includes.h"
#include "ipc_right.h"
#include "ipc_port.h"
#include "ipc_space.h"
#include "debug.h"
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/sched.h>

struct mach_port_right* ipc_right_new(darling_mach_port_t* port, mach_port_right_t type)
{
	struct mach_port_right* right;
	
	right = (struct mach_port_right*) kmalloc(sizeof(struct mach_port_right), GFP_KERNEL);
	if (right == NULL)
		return NULL;
	
	right->port = port;
	right->type = type;
	right->num_refs = 1;
	spin_lock_init(&right->port_lock);
	
	if (PORT_IS_VALID(port))
	{
		switch (type)
		{
			case MACH_PORT_RIGHT_RECEIVE:
			case MACH_PORT_RIGHT_PORT_SET:
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
	
	if (right->type == MACH_PORT_RIGHT_RECEIVE || right->type == MACH_PORT_RIGHT_PORT_SET)
	{
		debug_msg("\tCalling ipc_port_put()\n");
		ipc_port_put(port);
	}
	else if (right->type == MACH_PORT_RIGHT_SEND
		|| right->type == MACH_PORT_RIGHT_SEND_ONCE)
	{
		if (PORT_IS_VALID(port))
		{
			if (right->type == MACH_PORT_RIGHT_SEND)
			{
				port->num_srights--;
				debug_msg("\tChanging num of rights (s) -> %d\n", port->num_srights);
			}
			else if (right->type == MACH_PORT_RIGHT_SEND_ONCE)
			{
				port->num_sorights--;
				debug_msg("\tChanging num of rights (so) -> %d\n", port->num_sorights);
			}
			
			list_del(&right->reflist);
			
			// Special case for semaphores etc. where no receive port exists.
			// It cannot be applied to kernel server ports - these are
			// released explicitly.
			if (list_empty(&port->refs) && port->is_server_port
					&& port->server_port.subsystem == NULL)
			{
				ipc_port_put(port);
			}
		}
	}
	
	kfree(right);
	
	debug_msg("Deallocated right %p\n", right);
}

void ipc_right_put_unlock(struct mach_port_right* right)
{
	darling_mach_port_t* port;

	port = right->port;
	ipc_right_put(right);
	ipc_port_unlock(port);
}

void ipc_right_put_cloned_receive(struct mach_port_right* right)
{
	if (right == NULL)
		return;
	
	list_del(&right->reflist);
	kfree(right);
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

mach_msg_type_name_t ipc_right_copyin_type(mach_msg_type_name_t type)
{
	switch (type)
	{
		case MACH_MSG_TYPE_MAKE_SEND:
		case MACH_MSG_TYPE_COPY_SEND:
		case MACH_MSG_TYPE_MOVE_SEND:
			return MACH_MSG_TYPE_PORT_SEND;
			
		case MACH_MSG_TYPE_COPY_RECEIVE:
		case MACH_MSG_TYPE_MOVE_RECEIVE:
			return MACH_MSG_TYPE_PORT_RECEIVE;
		
		case MACH_MSG_TYPE_MAKE_SEND_ONCE:
		case MACH_MSG_TYPE_MOVE_SEND_ONCE:
			return MACH_MSG_TYPE_PORT_SEND_ONCE;
			
		default:
			return MACH_MSG_TYPE_PORT_NONE;
	}
}

void ipc_right_lock_port(struct mach_port_right* right)
{
	while (1)
	{
		spin_lock(&right->port_lock);

		if (!PORT_IS_VALID(right->port))
		{
			spin_unlock(&right->port_lock);
			break;
		}
		if (mutex_trylock(&right->port->mutex) != 0)
		{
			spin_unlock(&right->port_lock);
			break;
		}

		spin_unlock(&right->port_lock);
		schedule();
	}
}

