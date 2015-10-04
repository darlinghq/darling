#ifndef IPC_MSG_H
#define IPC_MSG_H
#include "ipc_types.h"
#include "mach_includes.h"

union complex_item
{
	darling_mach_port_right_t* port;
};
struct ipc_kmsg
{
	mach_msg_header_t* msg;
	darling_mach_port_right_t* target;
	darling_mach_port_right_t* reply;
	union complex_item* complex_items;
};

/**
 * Frees msg (either directly or deletion is left to the receiver).
 */
mach_msg_return_t ipc_msg_send(ipc_namespace_t* task, mach_msg_header_t* msg,
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
mach_msg_return_t ipc_msg_deliver(struct ipc_kmsg* kmsg,
		mach_msg_timeout_t timeout,
		int options);

mach_msg_return_t ipc_msg_recv(mach_task_t* task,
		mach_port_name_t port_name,
		mach_msg_header_t* msg,
		mach_msg_size_t receive_limit,
		mach_msg_timeout_t timeout,
		int options);

int ipc_msg_count(void);

#endif
