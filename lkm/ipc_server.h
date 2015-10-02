#ifndef IPC_SERVER_H
#define IPC_SERVER_H
#include "mach_includes.h"
#include "ipc_port.h"
#include "traps.h"

/*
 * The following functions associate a port with the named kernel server.
 * The port will no longer queue messages, instead the respective "Mach server"
 * will be invoked.
 */

void ipc_port_make_task(darling_mach_port_t* port, pid_t pid);
mach_task_t* ipc_port_get_task(darling_mach_port_t* port);

#endif
