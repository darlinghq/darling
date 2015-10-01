#include "mach_includes.h"
#include "ipc_server.h"
#include <linux/slab.h>
#include "task.h"
#include "debug.h"

static void task_free(server_port_t* kport)
{
	mach_task_t* task;
	
	task = (mach_task_t*) kport->private_data;
	
	/* Deallocate the port right space. Deletes all references. */
	ipc_space_put(&task->namespace);
	
	kfree(task);
}

void ipc_port_make_task(darling_mach_port_t* port, pid_t pid)
{
	mach_task_t* task;
	
	task = (mach_task_t*) kmalloc(sizeof(mach_task_t), GFP_KERNEL);
	task->pid = pid;
	task->task_self = port;
	
	ipc_space_init(&task->namespace);
	
	port->is_server_port = true;
	port->server_port.subsystem = MIG_SUBSYSTEM_NULL; // FIXME
	port->server_port.private_data = task;
	port->server_port.cb_free = task_free;
}

mach_task_t* ipc_port_get_task(darling_mach_port_t* port)
{
	if (!port->is_server_port)
		return NULL;
	// TODO: Compare mig_subsystem_t value instead
	//if (port->server_port.port_type != IPC_SERVER_TYPE_TASK)
	//	return NULL;
	
	return (mach_task_t*) port->server_port.private_data;
}

