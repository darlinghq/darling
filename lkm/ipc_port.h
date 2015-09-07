#ifndef IPC_PORT_H
#define IPC_PORT_H
#include "mach_includes.h"
#include <linux/mutex.h>
#include <linux/atomic.h>

typedef struct server_port server_port_t;
struct server_port
{
	/* E.g. IPC_SERVER_TYPE_TASK */
	int port_type;
	
	boolean_t (*cb_handler)(mach_msg_header_t* /* in */, mach_msg_header_t* /* out */);
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
		
		// TODO: members for message exchange
	};
};
typedef struct darling_mach_port darling_mach_port_t;

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

#endif
