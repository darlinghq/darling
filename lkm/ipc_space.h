#ifndef IPC_SPACE_H
#define IPC_SPACE_H
#include "ipc_right.h"
#include <linux/idr.h>
#include <linux/mutex.h>

struct mach_port_t;

struct ipc_space_t
{
	struct mutex mutex;
	/* Every allocated port right name comes from here */
	struct idr names;
};

void ipc_space_init(struct ipc_space_t* space);
void ipc_space_put(struct ipc_space_t* space);

/*
 * Finds the right corresponding to the given name in the given space.
 * 
 * Expects the space to be locked.
 * The returned right is locked (if not null or dead).
 */
struct mach_port_right* ipc_space_lookup(struct ipc_space_t* space, mach_port_name_t name);

/*
 * Locks the space. Expects the port to be locked.
 */
mach_msg_return_t ipc_space_make_receive(struct ipc_space_t* space, struct mach_port_t* port, mach_port_name_t* name_out);

/*
 * Locks the space. Expects the port to be locked.
 */
mach_msg_return_t ipc_space_make_send(struct ipc_space_t* space, struct mach_port_t* port, bool once, mach_port_name_t* name_out);

/*
 * Deallocate a port name.
 * 
 * Automatically deallocates port on last reference
 * or marks port as dead if name refers to a receive right.
 */
mach_msg_return_t ipc_space_right_put(struct ipc_space_t* space, mach_port_name_t name);

#endif
