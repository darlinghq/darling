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
#include "psynch_cv.h"
#include "../mach_includes.h"
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "../darling_task.h"
#include "../debug.h"
#include <stdbool.h>
#include <linux/jiffies.h>
#include "psynch_mutex.h"

struct pthread_cv
{
    struct hlist_node node;
    uint64_t pointer;
	wait_queue_head_t wq;
    struct list_head waiting;
    unsigned int refcount;
};
typedef struct pthread_cv pthread_cv_t;

struct pthread_waiter
{
	struct list_head entry;
	int wakeup;
};

static pthread_cv_t* cv_get(mach_task_t* task, uint64_t address,
		bool no_create);
static void cv_put(mach_task_t* task, pthread_cv_t* cv);

int psynch_cvwait_trap(mach_task_t* task,
		struct psynch_cvwait_args* in_args)
{
	struct psynch_cvwait_args args;
	struct pthread_waiter waiter;
	pthread_cv_t* cv;
	int retval;
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return -EFAULT;
	
	debug_msg("psynch_cvwait(): cv=%p, mutex=%p\n",
		(void*)args.cv, (void*)args.mutex);
	if (args.mutex != 0)
		psynch_mutexdrop(task, args.mutex, args.mgen, args.ugen);
	
	spin_lock(&task->cv_wq_lock);
    
	cv = cv_get(task, args.cv, false);
	BUG_ON(cv == NULL);
	
	waiter.wakeup = 0;
	list_add_tail(&waiter.entry, &cv->waiting);
	
	spin_unlock(&task->cv_wq_lock);
	
	// NOTE: args.usec seems to actually contain nsecs
	debug_msg("\tWaiting(%p)\n", &waiter);
	if (args.sec > 0 || args.usec > 0)
	{
		retval = wait_event_interruptible_timeout(cv->wq, waiter.wakeup != 0,
			usecs_to_jiffies(args.sec * USEC_PER_SEC + args.usec / 1000));
	}
	else
	{
		retval = wait_event_interruptible(cv->wq, waiter.wakeup != 0);
	}
	
	debug_msg("\tWoken up(%p)\n", &waiter);
	
	spin_lock(&task->cv_wq_lock);
	cv_put(task, cv);
	spin_unlock(&task->cv_wq_lock);
	
	return retval;
}

int psynch_cvbroad_trap(mach_task_t* task,
		struct psynch_cvbroad_args* in_args)
{
	struct psynch_cvbroad_args args;
	pthread_cv_t* cv;
	struct list_head* item;
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return -EFAULT;
	
	debug_msg("psynch_cvbroad_trap(): cv=%p\n", (void*) args.cv);
	spin_lock(&task->cv_wq_lock);
    
	cv = cv_get(task, args.cv, true);
	
	if (cv == NULL)
	{
		// Nobody is waiting
		spin_unlock(&task->cv_wq_lock);
		return 0;
	}
	
	list_for_each(item, &cv->waiting)
	{
		struct pthread_waiter* waiter;
		
		waiter = list_entry(item, typeof(*waiter), entry);
		debug_msg("\tWake up waiter %p\n", waiter);
		waiter->wakeup = 1;
	}
	INIT_LIST_HEAD(&cv->waiting);
	
	wake_up_interruptible(&cv->wq);
	cv_put(task, cv);
	spin_unlock(&task->cv_wq_lock);
	
	return 0x100 /* PTHRW_INC */;
}

int psynch_cvsignal_trap(mach_task_t* task,
		struct psynch_cvsignal_args* in_args)
{
	struct psynch_cvsignal_args args;
	pthread_cv_t* cv;
	struct pthread_waiter* waiter;
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return -EFAULT;
	
	debug_msg("psynch_cvsignal_trap(): cv=%p\n", (void*)args.cv);
	
	spin_lock(&task->cv_wq_lock);
	
	cv = cv_get(task, args.cv, true);
	
	if (cv == NULL)
	{
		// Nobody is waiting
		spin_unlock(&task->cv_wq_lock);
		return 0;
	}
	
	waiter = list_first_entry(&cv->waiting, typeof(*waiter), entry);
	waiter->wakeup = 1;
	debug_msg("\tWake up waiter %p\n", waiter);
	list_del(&waiter->entry);
	
	wake_up_interruptible(&cv->wq);
	cv_put(task, cv);
	spin_unlock(&task->cv_wq_lock);
	
	return 0x100 /* PTHRW_INC */;
}

pthread_cv_t* cv_get(mach_task_t* task, uint64_t address, bool no_create)
{
	pthread_cv_t* node;
	hash_for_each_possible(task->cv_wq, node, node, address)
	{
		if (node->pointer != address)
			continue;

		node->refcount++;
		return node;
	}
	
	if (no_create)
		return NULL;

	node = (pthread_cv_t*) kmalloc(sizeof(pthread_cv_t), GFP_KERNEL);
	node->refcount = 1;
	node->pointer = address;

	init_waitqueue_head(&node->wq);
	INIT_LIST_HEAD(&node->waiting);
	hash_add(task->cv_wq, &node->node, address);

	return node;
}

void cv_put(mach_task_t* task, pthread_cv_t* cv)
{
	cv->refcount--;

	if (cv->refcount == 0)
	{
		hash_del(&cv->node);
		kfree(cv);
	}
}
