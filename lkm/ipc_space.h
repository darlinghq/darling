#ifndef IPC_SPACE_H
#define IPC_SPACE_H
#include "ipc_types.h"
#include "ipc_right.h"
#include <linux/idr.h>
#include <linux/mutex.h>

struct mach_port_t;

struct ipc_space
{
	struct mutex mutex;
	/* Every allocated port right name comes from here */
	struct idr names;
};

void ipc_space_init(ipc_namespace_t* space);
void ipc_space_put(ipc_namespace_t* space);

void ipc_space_lock(ipc_namespace_t* space);
void ipc_space_unlock(ipc_namespace_t* space);

/*
 * Finds the right corresponding to the given name in the given space.
 * 
 * Expects the space to be locked.
 * The returned right is locked (if not null or dead).
 */
struct mach_port_right* ipc_space_lookup(ipc_namespace_t* space, mach_port_name_t name);

/*
 * Locks and unlocks the space. Expects the port to be locked.
 */
mach_msg_return_t ipc_space_make_receive(ipc_namespace_t* space, darling_mach_port_t* port, mach_port_name_t* name_out);

/*
 * Locks and unlocks the space. Expects the port to be locked.
 */
mach_msg_return_t ipc_space_make_send(ipc_namespace_t* space, darling_mach_port_t* port, bool once, mach_port_name_t* name_out);

/*
 * Inserts existing right into given namespace.
 * Expects the space to be locked.
 */
mach_msg_return_t ipc_space_right_insert(ipc_namespace_t* space, darling_mach_port_right_t* right, mach_port_name_t* name_out);

/*
 * Deallocate a port name.
 * 
 * Automatically deallocates port on last reference
 * or marks port as dead if name refers to a receive right.
 */
mach_msg_return_t ipc_space_right_put(ipc_namespace_t* space, mach_port_name_t name);

/*
 * Deallocate a port name. Does not delete the associated right.
 * Space is expected to be locked
 */
void ipc_space_name_put(ipc_namespace_t* space, mach_port_name_t name);

#endif
