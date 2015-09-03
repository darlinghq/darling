#ifndef IPC_SERVER_H
#define IPC_SERVER_H
#include "ipc_port.h"
#include "linuxmach.h"

#define IPC_SERVER_TYPE_HOST	1
#define IPC_SERVER_TYPE_TASK	2

/*
 * The following functions associate a port with the named kernel server.
 * The port will no longer queue messages, instead the respective "Mach server"
 * will be invoked.
 */

void ipc_port_make_task(darling_mach_port_t* port, pid_t pid);
mach_task_t* ipc_port_get_task(darling_mach_port_t* port);

void ipc_port_make_host(darling_mach_port_t* port);

#endif
