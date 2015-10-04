#ifndef IPC_PORT_H
#define IPC_PORT_H
#include "mach_includes.h"
#include "ipc_types.h"
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/wait.h>
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
	struct ipc_kmsg kmsg;
	
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

int ipc_port_count(void);

#endif
