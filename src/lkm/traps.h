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

#ifndef LINUXMACH_H
#define LINUXMACH_H
#include <linux/fs.h>
#include "ipc_space.h"
#include "api.h"

int mach_dev_open(struct inode* ino, struct file* file);
int mach_dev_release(struct inode* ino, struct file* file);
long mach_dev_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_paramv);

int mach_get_api_version(mach_task_t* task);
mach_port_name_t mach_reply_port_trap(mach_task_t* task);
kern_return_t _kernelrpc_mach_port_mod_refs_trap(mach_task_t* task,
		struct mach_port_mod_refs_args* args);
kern_return_t _kernelrpc_mach_port_deallocate_trap(mach_task_t* task,
		struct mach_port_deallocate_args* args);
kern_return_t _kernelrpc_mach_port_destroy_trap(mach_task_t* task,
		struct mach_port_destroy_args* args);
mach_port_name_t mach_task_self_trap(mach_task_t* task);
mach_port_name_t mach_thread_self_trap(mach_task_t* task);
mach_port_name_t mach_host_self_trap(mach_task_t* task);
kern_return_t _kernelrpc_mach_port_allocate_trap(mach_task_t* task,
												 struct mach_port_allocate_args* args);
kern_return_t mach_msg_overwrite_trap(mach_task_t* task,
		struct mach_msg_overwrite_args* args);

kern_return_t semaphore_signal_trap(mach_task_t* task,
		struct semaphore_signal_args* args);
kern_return_t semaphore_signal_all_trap(mach_task_t* task,
		struct semaphore_signal_all_args* args);
kern_return_t semaphore_wait_trap(mach_task_t* task,
		struct semaphore_wait_args* args);
kern_return_t semaphore_wait_signal_trap(mach_task_t* task,
		struct semaphore_wait_signal_args* args);
kern_return_t semaphore_timedwait_trap(mach_task_t* task,
		struct semaphore_timedwait_args* args);
kern_return_t semaphore_timedwait_signal_trap(mach_task_t* task,
		struct semaphore_timedwait_signal_args* args);
long bsd_ioctl_trap(mach_task_t* task, struct bsd_ioctl_args* fd);

// Internal
kern_return_t _kernelrpc_mach_port_destroy(mach_task_t* task,
		mach_port_name_t task_name, mach_port_name_t right_name);

#endif
