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
#include "../debug.h"
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <linux/list.h>

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
	
	debug_msg("mach_semaphore_signal(%p)\n", port);
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
	
	debug_msg("mach_semaphore_signal_all(%p)\n", port);
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
	
	debug_msg("mach_semaphore_wait(%p)\n", port);
	if (port->server_port.port_type != PORT_TYPE_SEMAPHORE)
	{
		ipc_port_unlock(port);
		return KERN_INVALID_ARGUMENT;
	}
	
	ms = (struct mach_semaphore*) port->server_port.private_data;
	
	read_lock(&ms->rwlock);
	ipc_port_unlock(port);
	
	ret = KERN_SUCCESS;
	
	if (down_interruptible(&ms->sem) == -EINTR)
	{
		ret = KERN_ABORTED;
	}
	
	if (!ms->active)
		ret = KERN_TERMINATED;
	
	read_unlock(&ms->rwlock);
	return ret;
}

// Copied from kernel/locking/semaphore.c (explanation below)
// This MUST MATCH the private definition in kernel!
struct semaphore_waiter {
	struct list_head list;
	struct task_struct *task;
	bool up;
};

// Copied from kernel/locking/semaphore.c because down_timeout_interruptible()
// doesn't exist.
// Otherwise, Darwin sleep() would not be interruptible by signals.
static inline int __down_common(struct semaphore *sem, long state,
                                                       long timeout) {
	struct task_struct *task = current;
	struct semaphore_waiter waiter;
 
	list_add_tail(&waiter.list, &sem->wait_list);
	waiter.task = task;
	waiter.up = false;

	for (;;) {
		if (signal_pending_state(state, task))
			goto interrupted;
		if (unlikely(timeout <= 0))
			goto timed_out;
		__set_task_state(task, state);
		raw_spin_unlock_irq(&sem->lock);
		timeout = schedule_timeout(timeout);
		raw_spin_lock_irq(&sem->lock);
		if (waiter.up)
			return 0;
	}
 
 timed_out:
	list_del(&waiter.list);
	return -ETIME;
 
interrupted:
	list_del(&waiter.list);
	return -EINTR;
}

static int __down_timeout_interruptible(struct semaphore *sem, long timeout)
{
	return __down_common(sem, TASK_INTERRUPTIBLE, timeout);
}

int down_timeout_interruptible(struct semaphore *sem, long timeout)
{
    unsigned long flags;
    int result = 0;

    raw_spin_lock_irqsave(&sem->lock, flags);
    if (likely(sem->count > 0))
        sem->count--;
    else
        result = __down_timeout_interruptible(sem, timeout);
    raw_spin_unlock_irqrestore(&sem->lock, flags);

    return result;
}


kern_return_t
mach_semaphore_timedwait(darling_mach_port_t* port, unsigned int sec,
		unsigned int nsec)
{
	struct mach_semaphore* ms;
	kern_return_t ret;
	int err;
	unsigned long jiffies;
	
	debug_msg("mach_semaphore_timedwait(%p)\n", port);
	if (port->server_port.port_type != PORT_TYPE_SEMAPHORE)
	{
		ipc_port_unlock(port);
		return KERN_INVALID_ARGUMENT;
	}
	
	ms = (struct mach_semaphore*) port->server_port.private_data;
	
	read_lock(&ms->rwlock);
	ipc_port_unlock(port);
	
	ret = KERN_SUCCESS;
	
	#if LINUX_VERSION_CODE < KERNEL_VERSION(3,17,0)
	jiffies = sec * HZ;
	jiffies += nsec / (NSEC_PER_SEC / HZ);
	#else
	jiffies = nsecs_to_jiffies(((u64) sec) * NSEC_PER_SEC + nsec);
	#endif
	
	debug_msg("down_timeout(jiffies=%lu, pid=%d)\n", jiffies, current->pid);
	err = down_timeout_interruptible(&ms->sem, jiffies);
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
