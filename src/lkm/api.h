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

#ifndef LKM_API_H
#define LKM_API_H
#include <stdint.h>

#define darling_mach_xstr(a) darling_mach_str(a)
#define darling_mach_str(a) #a

#define DARLING_MACH_API_VERSION		2
#define DARLING_MACH_API_VERSION_STR	darling_mach_xstr(DARLING_MACH_API_VERSION)

#define DARLING_MACH_API_BASE		0x1000

#pragma pack (push, 1)

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
	NR_semaphore_timedwait_trap,
	NR_bsd_ioctl_trap,
	NR_thread_self_trap,
	NR_bsdthread_terminate_trap,
	NR_psynch_mutexwait_trap, // 0x12
	NR_psynch_mutexdrop_trap,
	NR_pthread_kill_trap,
	NR_psynch_cvwait_trap, // 0x15
	NR_psynch_cvsignal_trap,
	NR_psynch_cvbroad_trap,
	NR_mk_timer_create_trap,
	NR_mk_timer_arm_trap,
	NR_mk_timer_cancel_trap,
	NR_mk_timer_destroy_trap,
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
#ifdef __i386__
	void* msg;
	uint32_t pad1;
#else
	void* msg;
#endif
	unsigned int option;
	unsigned int send_size;
	unsigned int recv_size;
	unsigned int rcv_name;
	unsigned int timeout;
	unsigned int notify;
#ifdef __i386__
	void* rcv_msg;
	uint32_t pad2;
#else
	void* rcv_msg;
#endif
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

struct bsd_ioctl_args
{
	int fd;
	uint64_t request;
	uint64_t arg;
};

struct bsdthread_terminate_args
{
	uint64_t stackaddr;
	uint32_t freesize;
	unsigned int thread_right_name;
	unsigned int signal;
};

struct psynch_mutexwait_args
{
	uint64_t mutex;
	uint32_t mgen;
	uint32_t ugen;
	uint64_t tid;
	uint32_t flags;
};

struct psynch_mutexdrop_args
{
	uint64_t mutex;
	uint32_t mgen;
	uint32_t ugen;
	uint64_t tid;
	uint32_t flags;
};

struct pthread_kill_args
{
	int thread_port;
	int sig;
};

struct psynch_cvwait_args
{
	uint64_t cv;
	uint32_t cvgen;
	uint32_t cvugen;
	uint64_t mutex;
	uint32_t mgen;
	uint32_t ugen;
	uint64_t sec;
	/* Called usec, but seems to contain nsec */
	uint64_t usec;
};

struct psynch_cvsignal_args
{
	uint64_t cv;
	uint32_t cvgen;
	uint32_t cvugen;
	uint64_t mutex;
	uint32_t mgen;
	uint32_t ugen;
	int thread_port;
	uint32_t flags;
};

struct psynch_cvbroad_args
{
	uint64_t cv;
	uint32_t cvgen;
	uint32_t diffgen;
	uint64_t mutex;
	uint32_t mgen;
	uint32_t ugen;
	uint64_t tid;
	uint32_t flags;
};

struct mk_timer_arm_args
{
	unsigned int timer_port;
	/* This is mach_absolute_time() based time */
	uint64_t expire_time;
};

struct mk_timer_cancel_args
{
	unsigned int timer_port;
	uint64_t* result_time;
#ifdef __i386__
	uint32_t pad1;
#endif
};

struct mk_timer_destroy_args
{
	unsigned int timer_port;
};

#pragma pack (pop)

#endif
