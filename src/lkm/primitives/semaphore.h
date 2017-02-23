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

#ifndef PRIMITIVES_SEMAPHORE_H
#define PRIMITIVES_SEMAPHORE_H
#include "../ipc_types.h"
#include <linux/semaphore.h>
#include <linux/list.h>
#include <linux/rwlock.h>

struct mach_semaphore
{
	bool active;
	struct semaphore sem;
	rwlock_t rwlock;
	mach_port_t kernel_right;
	struct list_head head;
};


void
mach_semaphore_setup(mach_task_t* task, darling_mach_port_t* port, int value, mach_port_t kernel_right);

// All of the following unlock the port on their own

kern_return_t
mach_semaphore_signal(darling_mach_port_t* port);

kern_return_t
mach_semaphore_signal_all(darling_mach_port_t* port);

kern_return_t
mach_semaphore_wait(darling_mach_port_t* port);

kern_return_t
mach_semaphore_timedwait(darling_mach_port_t* port, unsigned int sec,
		unsigned int nsec);

int mach_semaphore_count(void);

#endif
