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
#include "ipc_server.h"
#include <linux/slab.h>
#include <linux/rwlock.h>
#include "darling_task.h"
#include "debug.h"

extern struct mig_subsystem task_subsystem;

static void task_free(server_port_t* kport)
{
	mach_task_t* task;
	
	task = (mach_task_t*) kport->private_data;
	
	darling_task_free_threads(task);
	
	/* Deallocate the port right space. Deletes all references. */
	ipc_space_put(&task->namespace);
	
	kfree(task);
}

mach_task_t* ipc_port_make_task(darling_mach_port_t* port, pid_t pid)
{
	mach_task_t* task;
	
	task = (mach_task_t*) kmalloc(sizeof(mach_task_t), GFP_KERNEL);
	task->pid = pid;
	task->task_self = port;
	task->threads.rb_node = NULL;
	
	rwlock_init(&task->threads_lock);
    
    hash_init(task->mutex_wq);
    spin_lock_init(&task->mutex_wq_lock);
	
	ipc_space_init(&task->namespace);
	
	port->is_server_port = true;
	port->server_port.subsystem = &task_subsystem;
	port->server_port.private_data = task;
	port->server_port.cb_free = task_free;
	
	return task;
}

mach_task_t* ipc_port_get_task(darling_mach_port_t* port)
{
	if (!port->is_server_port)
		return NULL;
	// TODO: Compare mig_subsystem_t value instead
	//if (port->server_port.port_type != IPC_SERVER_TYPE_TASK)
	//	return NULL;
	
	return (mach_task_t*) port->server_port.private_data;
}

