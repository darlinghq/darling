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

#ifndef IPC_PORT_H
#define IPC_PORT_H
#include "mach_includes.h"
#include "ipc_types.h"
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/wait.h>
#include <linux/hashtable.h>
#include "ipc_msg.h"

typedef struct server_port server_port_t;
struct server_port
{
	/* Used for non-server special ports (e.g. semaphores) */
	int port_type;
	
	mig_subsystem_t subsystem;
	void (*cb_free)(server_port_t*);
	
	void* private_data;
};

struct darling_mach_port
{
	struct mutex mutex;
	
	struct list_head refs;

	int num_srights;
	int num_sorights;
	
	/* Whether this port is associated with a Mach server */
	bool is_server_port : 1;
	bool is_port_set : 1;
	
	union
	{
		/* If is_server_port is true */
		server_port_t server_port;
		
		/* Members for message exchange */
		struct
		{
			struct list_head messages;
			unsigned int queue_size;
			wait_queue_head_t queue_send, queue_recv;
			DECLARE_HASHTABLE(sets, 8);
		};
		// Port sets
		struct
		{
			DECLARE_HASHTABLE(members, 8);
		};
	};
};

// Special type for darling_mach_port list entries
// because ports may be in multiple port sets
struct darling_mach_port_le
{
	struct hlist_node node;
	struct darling_mach_port* port;
};

struct mach_port_right;

struct ipc_delivered_msg
{
	struct list_head list;
	struct ipc_kmsg kmsg;
	
	// Set to 1 after reception
	unsigned char delivered : 1;
	// Set to 1 if this struct is to be deleted by recipient
	unsigned char recipient_freed : 1;
	// When set to 1, the receiving port has died and says goodbye
	unsigned char recipient_died : 1;
};

mach_msg_return_t ipc_port_new(darling_mach_port_t** port);
mach_msg_return_t ipc_port_set_new(darling_mach_port_t** port);

// in/out: pset and port are locked
kern_return_t ipc_portset_insert(darling_mach_port_t* pset, darling_mach_port_t* port);

// in/out: pset and port are locked
kern_return_t ipc_portset_extract(darling_mach_port_t* pset, darling_mach_port_t* port);

// in/out: pset and port are locked, pset may be null
kern_return_t ipc_portset_move(darling_mach_port_t* pset, darling_mach_port_t* port);

/**
 * Deallocates the port. Marks all refering rights as PORT_DEAD.
 * @param port
 * @return 
 */
mach_msg_return_t ipc_port_put(darling_mach_port_t* port);

/**
 * Locks the port's mutex.
 * Checks for null and dead ports.
 */
void ipc_port_lock(darling_mach_port_t* port);

/**
 * Unlocks the port's mutex.
 * Checks for null and dead ports.
 */
void ipc_port_unlock(darling_mach_port_t* port);

int ipc_port_count(void);

#endif
