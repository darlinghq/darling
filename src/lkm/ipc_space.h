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

#ifndef IPC_SPACE_H
#define IPC_SPACE_H
#include "ipc_types.h"
#include "ipc_right.h"
#include <linux/idr.h>
#include <linux/mutex.h>

struct mach_port_t;

struct ipc_space
{
	struct mutex mutex;
	/* Every allocated port right name comes from here */
	struct idr names;
};

void ipc_space_init(ipc_namespace_t* space);
void ipc_space_put(ipc_namespace_t* space);

void ipc_space_lock(ipc_namespace_t* space);
void ipc_space_unlock(ipc_namespace_t* space);

/*
 * Finds the right corresponding to the given name in the given space.
 * 
 * Expects the space to be locked.
 * The returned right is locked (if not null or dead).
 */
struct mach_port_right* ipc_space_lookup(ipc_namespace_t* space, mach_port_name_t name);

/*
 * Finds the right corresponding to the given name in the given space.
 * 
 * Expects the space to be locked.
 * The returned right is not locked.
 */
struct mach_port_right* ipc_space_lookup_unlocked(ipc_namespace_t* space, mach_port_name_t name);

/*
 * Locks and unlocks the space. Expects the port to be locked.
 */
mach_msg_return_t ipc_space_make_receive(ipc_namespace_t* space, darling_mach_port_t* port, mach_port_name_t* name_out);

/*
 * Locks and unlocks the space. Expects the port to be locked.
 */
mach_msg_return_t ipc_space_make_send(ipc_namespace_t* space, darling_mach_port_t* port, bool once, mach_port_name_t* name_out);

/*
 * Inserts existing right into given namespace.
 * Expects the space to be locked.
 */
mach_msg_return_t ipc_space_right_insert(ipc_namespace_t* space, darling_mach_port_right_t* right, mach_port_name_t* name_out);

/*
 * Deallocate a port name.
 * 
 * Automatically deallocates port on last reference
 * or marks port as dead if name refers to a receive right.
 */
mach_msg_return_t ipc_space_right_put(ipc_namespace_t* space, mach_port_name_t name);

/*
 * Deallocate a port name. Does not delete the associated right.
 * Space is expected to be locked
 */
void ipc_space_name_put(ipc_namespace_t* space, mach_port_name_t name);

#endif
