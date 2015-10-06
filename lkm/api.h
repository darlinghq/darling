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

#ifndef LKM_API_H
#define LKM_API_H

#define DARLING_MACH_API_VERSION		1
#define DARLING_MACH_API_VERSION_STR	"1"

#define DARLING_MACH_API_BASE		0x1000

enum { NR_get_api_version = DARLING_MACH_API_BASE,
	NR_mach_reply_port,
	NR__kernelrpc_mach_port_mod_refs,
	NR_task_self_trap,
	NR_host_self_trap,
	NR__kernelrpc_mach_port_allocate,
	NR_mach_msg_overwrite_trap,
	NR__kernelrpc_mach_port_deallocate,
	NR__kernelrpc_mach_port_destroy,
	NR_semaphore_signal_trap,
	NR_semaphore_signal_all_trap,
	NR_semaphore_wait_trap,
	NR_semaphore_wait_signal_trap,
	NR_semaphore_timedwait_signal_trap,
	NR_semaphore_timedwait_trap
};

struct mach_port_mod_refs_args
{
	unsigned int task_right_name;
	unsigned int port_right_name;
	int right_type;
	int delta;
};

struct mach_port_allocate_args
{
	unsigned int task_right_name;
	int right_type;
	unsigned int out_right_name;
};

struct mach_msg_overwrite_args
{
	union { void* msg; long long pad; };
	unsigned int option;
	unsigned int send_size;
	unsigned int recv_size;
	unsigned int rcv_name;
	unsigned int timeout;
	unsigned int notify;
	union { void* rcv_msg; long long pad2; };
	unsigned int rcv_limit;
};

struct mach_port_deallocate_args
{
	unsigned int task_right_name;
	unsigned int port_right_name;
};

struct mach_port_destroy_args
{
	unsigned int task_right_name;
	unsigned int port_right_name;
};

struct semaphore_signal_args
{
	unsigned int signal;
};

struct semaphore_signal_all_args
{
	unsigned int signal;
};

struct semaphore_wait_args
{
	unsigned int signal;
};

struct semaphore_wait_signal_args
{
	unsigned int wait;
	unsigned int signal;
};

struct semaphore_timedwait_signal_args
{
	unsigned int wait;
	unsigned int signal;
	unsigned int sec;
	unsigned int nsec;
};

struct semaphore_timedwait_args
{
	unsigned int wait;
	unsigned int sec;
	unsigned int nsec;
};

#endif
