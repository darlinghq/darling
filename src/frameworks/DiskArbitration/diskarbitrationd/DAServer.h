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

// diskarbitrationd's XPC service (see DAProtocol.h). Everything runs on the daemon's queue.
#ifndef DA_SERVER_H
#define DA_SERVER_H

#include <dispatch/dispatch.h>
#include <stdbool.h>
#include <xpc/xpc.h>

#include "DAModel.h"

// Starts listening on the DiskArbitration Mach service; requests are handled on queue.
void DAServerStart(dispatch_queue_t queue);

// What model changes are reported to (the server's clients).
const DAModelEvents* DAServerEvents(void);

// The daemon's queue.
dispatch_queue_t DAServerQueue(void);

// Calls block for every connection that said hello.
void DAServerForEachSubscriber(void (^block)(xpc_connection_t peer));

// Puts a description into a message as DA_KEY_DESCRIPTION (see DAProtocol.h); false if it cannot be encoded.
bool DAServerSetDescription(xpc_object_t message, CFDictionaryRef description);

#endif
