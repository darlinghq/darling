/*
 * Darling Mach Linux Kernel Module
 * Copyright (C) 2015-2017 Lubos Dolezel
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

#ifndef _TASK_H
#define _TASK_H
#include "ipc_types.h"
#include "ipc_space.h"
#include "ipc_port.h"
#include <linux/thread_info.h>
#include <linux/rbtree.h>
#include <linux/spinlock.h>
#include <linux/hashtable.h>
#include <linux/list.h>

// Initialized in ipc_server.c
struct mach_task
{
	pid_t pid;
	ipc_namespace_t namespace;
	
	// TODO: add bootstrap port
	darling_mach_port_t* task_self;
	
	rwlock_t threads_lock;
	struct rb_root threads;
	
	spinlock_t mutex_wq_lock;
	DECLARE_HASHTABLE(mutex_wq, 8);
	
	spinlock_t cv_wq_lock;
	DECLARE_HASHTABLE(cv_wq, 8);

	struct list_head semaphores;
};

typedef struct mach_task mach_task_t;

/* Global initialization */
void
darling_task_init(void);

mach_task_t*
darling_task_get_current(void);

void
darling_task_set_current(mach_task_t* task);

unsigned int
darling_get_task_count(void);

void darling_task_register_thread(mach_task_t* task,
		darling_mach_port_t* thread_port);
void darling_task_deregister_thread(mach_task_t* task,
		darling_mach_port_t* thread_port);
darling_mach_port_t* darling_task_lookup_thread(mach_task_t* task,
		pid_t thread);

void darling_task_destruct(mach_task_t* task);

static inline bool
task_is_64bit(void)
{
#if defined(__i386__)
	return false;
#elif defined(__x86_64__)
	return !test_thread_flag(TIF_IA32);
#endif
}

#endif
