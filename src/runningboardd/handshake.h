/**
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Darling developers
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

#ifndef RUNNINGBOARDD_HANDSHAKE_H
#define RUNNINGBOARDD_HANDSHAKE_H

#include <xpc/xpc.h>
#include <xpc/connection.h>

#define HANDSHAKE_SELECTOR "handshakeWithRequest:"

// Builds the value of the reply's "return-value" key for the given caller.
xpc_object_t handshake_response_create(xpc_connection_t caller);

#endif
