#ifndef IPC_PORT_H
#define IPC_PORT_H
#include "mach_includes.h"
#include "ipc_types.h"
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

	int num_srights;
	int num_sorights;
	
	/* Whether this port is associated with a Mach server */
	bool is_server_port;
	
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
		};
	};
};

struct mach_port_right;

struct ipc_delivered_msg
{
	struct list_head list;
	mach_msg_header_t* msg;
	darling_mach_port_right_t* reply;
	darling_mach_port_right_t** ool_ports;
	
	// Set to 1 after reception
	unsigned char delivered : 1;
	// Set to 1 if this struct is to be deleted by recipient
	unsigned char recipient_freed : 1;
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

/**
 * Frees msg (either directly or deletion is left to the receiver).
 */
mach_msg_return_t ipc_msg_send(mach_task_t* task, mach_msg_header_t* msg,
		mach_msg_timeout_t timeout, int options);

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
		mach_msg_timeout_t timeout,
		int options);

mach_msg_return_t ipc_msg_recv(mach_task_t* task,
		mach_port_name_t port_name,
		mach_msg_header_t* msg,
		mach_msg_size_t receive_limit,
		mach_msg_timeout_t timeout,
		int options);

#endif
