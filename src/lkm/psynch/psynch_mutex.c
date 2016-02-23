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

#include "psynch_mutex.h"
#include "../darling_task.h"
#include "../debug.h"
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

struct pthread_mutex
{
    struct hlist_node node;
    uint64_t pointer;
    uint32_t mgen;
	wait_queue_head_t wq;
    struct list_head waiting;
    unsigned int refcount;
	bool underlock; // unlocked when no one was waiting
};
typedef struct pthread_mutex pthread_mutex_t;

struct pthread_waiter
{
	struct list_head entry;
	int wakeup;
};

static pthread_mutex_t* mutex_get(mach_task_t* task, uint64_t address);
static void mutex_put(mach_task_t* task, pthread_mutex_t* mutex);

int psynch_mutexwait_trap(mach_task_t* task,
		struct psynch_mutexwait_args* in_args)
{
	struct psynch_mutexwait_args args;
	struct pthread_waiter waiter;
	pthread_mutex_t* mutex;
	int retval = 0;
    
	if (copy_from_user(&args, in_args, sizeof(args)))
		return -EFAULT;
    
	debug_msg("psynch_mutexwait_trap(%d): mutex=0x%llx, mgen=0x%x\n",
			current->pid, args.mutex, args.mgen);
	spin_lock(&task->mutex_wq_lock);
    
	mutex = mutex_get(task, args.mutex);
    
	// TODO: what if this destroys the mutex whilst someone else is waiting?
	if (mutex->mgen != 0 && mutex->underlock)
	{
		retval = mutex->mgen;
		
		// Put the mutex twice, because the waker did not put the mutex
		// in order to keep the information around
		mutex_put(task, mutex);
		mutex_put(task, mutex);
		
		spin_unlock(&task->mutex_wq_lock);

		goto out;
	}
	
	waiter.wakeup = 0;
	list_add_tail(&waiter.entry, &mutex->waiting);
	
	spin_unlock(&task->mutex_wq_lock);
	
	retval = wait_event_interruptible(mutex->wq, waiter.wakeup != 0);
	
	spin_lock(&task->mutex_wq_lock);
	list_del(&waiter.entry);
	
	if (waiter.wakeup)
	{
		retval = mutex->mgen;
		mutex->mgen = 0;
	}
	debug_msg("(%d)--> retval: 0x%x\n", current->pid, retval);
	
	mutex_put(task, mutex);
	spin_unlock(&task->mutex_wq_lock);
	
out:
	return retval;
}

int psynch_mutexdrop_trap(mach_task_t* task,
		struct psynch_mutexdrop_args* in_args)
{
	struct psynch_mutexdrop_args args;

	if (copy_from_user(&args, in_args, sizeof(args)))
		return -EFAULT;

	return psynch_mutexdrop(task, args.mutex, args.mgen, args.ugen);
}

int psynch_mutexdrop(mach_task_t* task, uint64_t in_mutex, uint32_t mgen,
		uint32_t ugen)
{
	pthread_mutex_t* mutex;
	
	debug_msg("psynch_mutexdrop(%d): mutex=0x%llx, mgen=0x%x\n",
			current->pid, in_mutex, mgen);
	spin_lock(&task->mutex_wq_lock);
    
	mutex = mutex_get(task, in_mutex);
	mutex->mgen = mgen;

	if (!list_empty(&mutex->waiting))
	{
		struct pthread_waiter* waiter;

		waiter = list_first_entry(&mutex->waiting, struct pthread_waiter,
				entry);

		waiter->wakeup = 1;

		wake_up_interruptible(&mutex->wq);
		mutex_put(task, mutex);
	} else
		mutex->underlock = true;

	spin_unlock(&task->mutex_wq_lock);

    return 0;
}

pthread_mutex_t* mutex_get(mach_task_t* task, uint64_t address)
{
	pthread_mutex_t* node;
	hash_for_each_possible(task->mutex_wq, node, node, address)
	{
		if (node->pointer != address)
			continue;

		node->refcount++;
		return node;
	}

	node = (pthread_mutex_t*) kmalloc(sizeof(pthread_mutex_t), GFP_KERNEL);
	node->refcount = 1;
	node->mgen = 0;
	node->pointer = address;
	node->underlock = false;

	init_waitqueue_head(&node->wq);
	INIT_LIST_HEAD(&node->waiting);
	hash_add(task->mutex_wq, &node->node, address);

	return node;
}

void mutex_put(mach_task_t* task, pthread_mutex_t* mutex)
{
	mutex->refcount--;

	if (mutex->refcount == 0)
	{
		debug_msg("Destroying mutex %p", mutex);
		hash_del(&mutex->node);
		kfree(mutex);
	}
	
	if (mutex->refcount < 0)
	{
		debug_msg("!!!!!!! refcount is %d", mutex->refcount);
	}
}
