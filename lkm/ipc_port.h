#ifndef IPC_PORT_H
#define IPC_PORT_H
#include "mach_includes.h"
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/wait.h>

typedef struct server_port server_port_t;
struct server_port
{
	/* E.g. IPC_SERVER_TYPE_TASK */
	int port_type;
	
	mig_subsystem_t subsystem;
	void (*cb_free)(server_port_t*);
	
	void* private_data;
};

struct darling_mach_port
{
	struct mutex mutex;
	
	struct list_head refs;
	struct list_head messages;
	wait_queue_head_t queue_send, queue_recv;

	int num_srights;
	int num_sorights;
	
	/* Whether this port is associated with a Mach server */
	bool is_server_port;
	
	union
	{
		/* If is_server_port is true */
		server_port_t server_port;
		
		// TODO: members for message exchange
	};
};
typedef struct darling_mach_port darling_mach_port_t;
typedef struct mach_task mach_task_t;
struct mach_port_right;

struct ipc_delivered_msg
{
	struct list_head list;
	mach_msg_header_t* msg;
	bool delivered;
};

mach_msg_return_t ipc_port_new(darling_mach_port_t** port);

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

mach_msg_return_t ipc_msg_send(mach_task_t* task, mach_msg_header_t* msg, mach_msg_timeout_t timeout);

/**
 * Enqueues the given message for reception.
 * 
 * For kernel server ports, the message will be processed immediately.
 * For userspace ports, the message will be enqueued and the call will block
 * until timeout expires.
 * 
 * @param msg Message to be delivered.
 * @param target Target port/right where to deliver the message.
 * @param reply Reply port/right.
 * @param timeout Timeout (or MACH_MSG_TIMEOUT_NONE).
 */
mach_msg_return_t ipc_msg_deliver(mach_msg_header_t* msg,
		struct mach_port_right* target,
		struct mach_port_right* reply,
		mach_msg_timeout_t timeout);

#endif
