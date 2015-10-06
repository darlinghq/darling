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

#include "../mach_includes.h"
#include "semaphore.h"
#include "../ipc_server.h"
#include "../ipc_port.h"
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/atomic.h>

static atomic_t sem_count = ATOMIC_INIT(0);

static void
mach_semaphore_destroy(server_port_t* port);

struct mach_semaphore
{
	bool active;
	struct semaphore sem;
	rwlock_t rwlock;
};

void
mach_semaphore_setup(darling_mach_port_t* port, int value)
{
	struct mach_semaphore* ms;
	
	port->is_server_port = true;
	port->server_port.port_type = PORT_TYPE_SEMAPHORE;
	port->server_port.subsystem = NULL;
	port->server_port.cb_free = mach_semaphore_destroy;
	
	ms = (struct mach_semaphore*) kmalloc(sizeof(struct mach_semaphore),
			GFP_KERNEL);
	port->server_port.private_data = ms;
	
	ms->active = true;
	sema_init(&ms->sem, value);
	rwlock_init(&ms->rwlock);
	
	atomic_inc(&sem_count);
}

static void
mach_semaphore_destroy(server_port_t* port)
{
	struct mach_semaphore* ms;
	
	ms = (struct mach_semaphore*) port->private_data;
	ms->active = false;
	
	while (!write_trylock(&ms->rwlock))
		up(&ms->sem);
	
	kfree(port->private_data);
	port->private_data = NULL;
	
	atomic_dec(&sem_count);
}

kern_return_t
mach_semaphore_signal(darling_mach_port_t* port)
{
	struct mach_semaphore* ms;
	
	if (port->server_port.port_type != PORT_TYPE_SEMAPHORE)
	{
		ipc_port_unlock(port);
		return KERN_INVALID_ARGUMENT;
	}
	
	ms = (struct mach_semaphore*) port->server_port.private_data;
	
	read_lock(&ms->rwlock);
	ipc_port_unlock(port);
	
	up(&ms->sem);
	read_unlock(&ms->rwlock);
	
	return KERN_SUCCESS;
}

kern_return_t
mach_semaphore_signal_all(darling_mach_port_t* port)
{
	struct mach_semaphore* ms;
	
	if (port->server_port.port_type != PORT_TYPE_SEMAPHORE)
	{
		ipc_port_unlock(port);
		return KERN_INVALID_ARGUMENT;
	}
	
	ms = (struct mach_semaphore*) port->server_port.private_data;
	
	read_lock(&ms->rwlock);
	ipc_port_unlock(port);
	
	while (ms->sem.count < 0)
		up(&ms->sem);
	
	read_unlock(&ms->rwlock);
	return KERN_SUCCESS;
}

kern_return_t
mach_semaphore_wait(darling_mach_port_t* port)
{
	struct mach_semaphore* ms;
	kern_return_t ret;
	
	if (port->server_port.port_type != PORT_TYPE_SEMAPHORE)
	{
		ipc_port_unlock(port);
		return KERN_INVALID_ARGUMENT;
	}
	
	ms = (struct mach_semaphore*) port->server_port.private_data;
	
	read_lock(&ms->rwlock);
	ipc_port_unlock(port);
	
	ret = KERN_SUCCESS;
	
	if (down_killable(&ms->sem) == -EINTR)
	{
		ret = KERN_ABORTED;
	}
	
	if (!ms->active)
		ret = KERN_TERMINATED;
	
	read_unlock(&ms->rwlock);
	return ret;
}

kern_return_t
mach_semaphore_timedwait(darling_mach_port_t* port, unsigned int sec,
		unsigned int nsec)
{
	struct mach_semaphore* ms;
	kern_return_t ret;
	int err;
	unsigned long jiffies;
	
	if (port->server_port.port_type != PORT_TYPE_SEMAPHORE)
	{
		ipc_port_unlock(port);
		return KERN_INVALID_ARGUMENT;
	}
	
	ms = (struct mach_semaphore*) port->server_port.private_data;
	
	read_lock(&ms->rwlock);
	ipc_port_unlock(port);
	
	ret = KERN_SUCCESS;
	
	jiffies = nsecs_to_jiffies(((u64) sec) * 1000000000ull + nsec);
	
	err = down_timeout(&ms->sem, jiffies);
	if (err == -ETIME)
		ret = KERN_OPERATION_TIMED_OUT;
	else if (err == -EINTR)
		ret = KERN_ABORTED;
	
	if (!ms->active)
		ret = KERN_TERMINATED;
	
	read_unlock(&ms->rwlock);
	return ret;
}

int mach_semaphore_count(void)
{
	return atomic_read(&sem_count);
}
