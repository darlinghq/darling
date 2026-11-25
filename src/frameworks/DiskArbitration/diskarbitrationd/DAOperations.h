/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// Unmount and eject requests (DAProtocol.h), with their approval rounds. On the daemon's queue.
#ifndef DA_OPERATIONS_H
#define DA_OPERATIONS_H

#include <xpc/xpc.h>

// Each takes the request and a reply made for it, and sends the reply (retaining it) once the request is done.
void DAOperationUnmount(xpc_connection_t peer, xpc_object_t message, xpc_object_t reply);
void DAOperationEject(xpc_connection_t peer, xpc_object_t message, xpc_object_t reply);

#endif
