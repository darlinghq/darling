/*
 * Darling Mach Linux Kernel Module
 * Copyright (C) 2015 Lubos Dolezel
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef IPC_SERVER_H
#define IPC_SERVER_H
#include "mach_includes.h"
#include "ipc_port.h"
#include "traps.h"

#define PORT_TYPE_SEMAPHORE		1

/*
 * The following functions associate a port with the named kernel server.
 * The port will no longer queue messages, instead the respective "Mach server"
 * will be invoked.
 */

void ipc_port_make_task(darling_mach_port_t* port, pid_t pid);
mach_task_t* ipc_port_get_task(darling_mach_port_t* port);

#endif
