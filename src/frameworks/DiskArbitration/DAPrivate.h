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

// The library's own declarations, shared by its source files.
#ifndef DA_PRIVATE_H
#define DA_PRIVATE_H

#include <CoreFoundation/CFRuntime.h>
#include <DiskArbitration/DiskArbitration.h>
#include <dispatch/dispatch.h>
#include <stdbool.h>
#include <xpc/xpc.h>

#define DA_HIDDEN __attribute__((visibility("hidden")))

struct __DASession
{
	CFRuntimeBase _parent;
	dispatch_queue_t queue;       // the connection's own queue
	xpc_connection_t connection;  // to diskarbitrationd
};

struct __DADisk
{
	CFRuntimeBase _parent;
	DASessionRef session;
	CFStringRef name; // the BSD name
	char bsdName[64];
};

// DAClient.c: requests to diskarbitrationd, answered synchronously. NULL if the daemon cannot be reached or
// sent no reply.
DA_HIDDEN xpc_object_t _DAClientRequest(DASessionRef session, xpc_object_t message);

// The description of the disk with that BSD name, as the library returns it, or NULL if there is no such disk.
DA_HIDDEN CFDictionaryRef _DAClientCopyDescription(DASessionRef session, const char* name);

// The BSD name of the volume mounted exactly on path (an absolute path without symbolic links), or false.
DA_HIDDEN bool _DAClientLookupPath(DASessionRef session, const char* path, char* name, size_t size);

// DASession.c
DA_HIDDEN void _DASessionConnect(DASessionRef session);

#endif
