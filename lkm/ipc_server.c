#include "mach_includes.h"
#include "ipc_server.h"
#include <linux/slab.h>
#include "debug.h"

static boolean_t dummy_task_server_handler(mach_msg_header_t* in, mach_msg_header_t* out)
{
	debug_msg("dummy_task_server_handler() invoked, msg=%p\n", in);
	return false;
}

static boolean_t dummy_host_server_handler(mach_msg_header_t* in, mach_msg_header_t* out)
{
	debug_msg("dummy_host_server_handler() invoked, msg=%p\n", in);
	return false;
}

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
	port->server_port.port_type = IPC_SERVER_TYPE_TASK;
	port->server_port.cb_handler = dummy_task_server_handler;
	port->server_port.private_data = task;
	port->server_port.cb_free = task_free;
}

mach_task_t* ipc_port_get_task(darling_mach_port_t* port)
{
	if (!port->is_server_port)
		return NULL;
	if (port->server_port.port_type != IPC_SERVER_TYPE_TASK)
		return NULL;
	
	return (mach_task_t*) port->server_port.private_data;
}

void ipc_port_make_host(darling_mach_port_t* port)
{
	port->is_server_port = true;
	port->server_port.port_type = IPC_SERVER_TYPE_HOST;
	port->server_port.cb_handler = dummy_host_server_handler;
	port->server_port.cb_free = NULL;
}
