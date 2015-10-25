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

#include "darling_task.h"
#include "debug.h"
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/printk.h>
#include <linux/rwlock.h>
#include "servers/thread_act.h"

static rwlock_t my_lock;
static struct rb_root all_tasks = RB_ROOT;
static unsigned int task_count = 0;

struct task_entry
{
	struct rb_node node;
	int tgid;
	mach_task_t* task;
};

void
darling_task_init(void)
{
	rwlock_init(&my_lock);
}

unsigned int
darling_get_task_count(void)
{
	return task_count;
}

mach_task_t*
darling_task_get_current(void)
{
	mach_task_t* ret = NULL;
	struct rb_node* node;
	
	read_lock(&my_lock);
	
	node = all_tasks.rb_node;
	
	while (node)
	{
		struct task_entry* entry = container_of(node, struct task_entry, node);
		
		if (current->tgid < entry->tgid)
			node = node->rb_left;
		else if (current->tgid > entry->tgid)
			node = node->rb_right;
		else
		{
			ret = entry->task;
			break;
		}
	}
	
	read_unlock(&my_lock);
	return ret;
}

void
darling_task_set_current(mach_task_t* task)
{
	struct task_entry* entry;
	struct rb_node **new, *parent = NULL;
	
	entry = (struct task_entry*) kmalloc(sizeof(struct task_entry), GFP_KERNEL);
	entry->task = task;
	entry->tgid = current->tgid;
	
	write_lock(&my_lock);
	new = &all_tasks.rb_node;
	
	while (*new)
	{
		struct task_entry* this = container_of(*new, struct task_entry, node);
		parent = *new;
		
		if (entry->tgid < this->tgid)
			new = &(*new)->rb_left;
		else if (entry->tgid > this->tgid)
			new = &(*new)->rb_right;
		else // Tree already contains this tgid
		{
			kfree(entry);
			
			if (task != NULL)
			{
				// Overwrite existing task entry
				debug_msg("darling_task_set_current() called twice with "
						"non-null task?!\n");
				
				this->task = task;
			}
			else
			{
				// Remove task from tree
				rb_erase(*new, &all_tasks);
				kfree(this);
				task_count--;
			}
			
			write_unlock(&my_lock);
			return;
		}
	}
	
	rb_link_node(&entry->node, parent, new);
	rb_insert_color(&entry->node, &all_tasks);
	task_count++;
	
	write_unlock(&my_lock);
}

struct thread_entry
{
	struct rb_node node;
	darling_mach_port_t* thread_port;
};

void darling_task_register_thread(mach_task_t* task,
		darling_mach_port_t* thread_port)
{
	struct thread_entry* entry;
	struct rb_node **new, *parent = NULL;
	pid_t my_pid;
	
	my_pid = get_thread_pid(thread_port);
	
	write_lock(&task->threads_lock);

	entry = (struct thread_entry*) kmalloc(sizeof(struct thread_entry),
		GFP_KERNEL);
	entry->thread_port = thread_port;
	
	new = &task->threads.rb_node;
	
	while (*new)
	{
		struct thread_entry* this = container_of(*new,
				struct thread_entry, node);
		parent = *new;
		
		if (my_pid < get_thread_pid(this->thread_port))
			new = &(*new)->rb_left;
		else if (my_pid > get_thread_pid(this->thread_port))
			new = &(*new)->rb_right;
		else
		{
			debug_msg("darling_task_register_thread(): "
					"PID already registered (%d)!\n", my_pid);
			
			write_unlock(&task->threads_lock);
			kfree(entry);
			return;
		}
	}
	
	rb_link_node(&entry->node, parent, new);
	rb_insert_color(&entry->node, &task->threads);
	
	write_unlock(&task->threads_lock);
}

void darling_task_deregister_thread(mach_task_t* task,
		darling_mach_port_t* thread_port)
{
	struct rb_node *node;
	pid_t my_pid;
	
	my_pid = get_thread_pid(thread_port);
	
	write_lock(&task->threads_lock);
	
	node = task->threads.rb_node;
	
	while (node)
	{
		struct thread_entry* entry = container_of(node,
				struct thread_entry, node);
		
		if (my_pid < get_thread_pid(entry->thread_port))
			node = node->rb_left;
		else if (my_pid > get_thread_pid(entry->thread_port))
			node = node->rb_right;
		else
		{
			rb_erase(node, &task->threads);
			kfree(entry);
			break;
		}
	}
	
	write_unlock(&task->threads_lock);
}

void darling_task_free_threads(mach_task_t* task)
{
	struct rb_node *node, *next;
	struct thread_entry* entry;
	
	write_lock(&task->threads_lock);
	
	node = rb_first(&task->threads);
	while (node != NULL)
	{
		entry = container_of(node, struct thread_entry, node);
		
		ipc_port_put(entry->thread_port);
		
		next = rb_next(node);
		
		kfree(entry);
		node = next;
	}
	
	task->threads.rb_node = NULL;
	write_unlock(&task->threads_lock);
}

darling_mach_port_t* darling_task_lookup_thread(mach_task_t* task,
		pid_t pid)
{
	struct rb_node *node;
	darling_mach_port_t* ret = NULL;
	
	read_lock(&task->threads_lock);
	node = task->threads.rb_node;
	
	while (node)
	{
		struct thread_entry* entry = container_of(node,
				struct thread_entry, node);
		
		if (pid < get_thread_pid(entry->thread_port))
			node = node->rb_left;
		else if (pid > get_thread_pid(entry->thread_port))
			node = node->rb_right;
		else
		{
			ret = entry->thread_port;
			break;
		}
	}
	
	read_unlock(&task->threads_lock);
	return ret;
}
