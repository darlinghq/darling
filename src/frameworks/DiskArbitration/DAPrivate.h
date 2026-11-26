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
#include <pthread.h>
#include <stdbool.h>
#include <xpc/xpc.h>

#define DA_HIDDEN __attribute__((visibility("hidden")))

struct DASessionLink;
struct DACallback;
struct DACallout;

struct __DASession
{
	CFRuntimeBase _parent;
	dispatch_queue_t queue;       // the connection's own queue
	xpc_connection_t connection;  // to diskarbitrationd
	struct DASessionLink* link;   // how the connection's handlers reach the session

	// Used only on queue: whether the reply to the last hello is still to come, its number, and the events that
	// arrived before it.
	bool helloPending;
	unsigned helloNumber;
	xpc_object_t earlyEvents;

	pthread_mutex_t lock; // guards the rest
	CFMutableArrayRef diskNames;  // the disks the session knows, in the order they appeared
	CFMutableDictionaryRef disks; // their last descriptions, by name
	struct DACallback* callbacks;
	struct DACallout* callouts;   // to be delivered on the run loops or the dispatch queue
	struct DACallout** calloutsTail;
	CFRunLoopSourceRef source;
	CFMutableArrayRef schedules;  // the run loops and modes the source is in, as two-element arrays
	dispatch_queue_t target;      // DASessionSetDispatchQueue
	bool selfRetained;            // a scheduled session keeps itself alive
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

// A description in an XPC message (DA_KEY_DESCRIPTION), as the library returns it, or NULL.
DA_HIDDEN CFMutableDictionaryRef _DAClientDescriptionFromMessage(xpc_object_t message);

// The BSD name of the volume mounted exactly on path (an absolute path without symbolic links), or false.
DA_HIDDEN bool _DAClientLookupPath(DASessionRef session, const char* path, char* name, size_t size);

// DASessionEvents.c: the session's connection, its disks and callbacks.
DA_HIDDEN void _DASessionConnect(DASessionRef session);
DA_HIDDEN void _DASessionDisconnect(DASessionRef session);

// Sends a DA_OP_UNMOUNT or DA_OP_EJECT request; function (a DADiskUnmountCallback, DADiskEjectCallback or
// DADiskMountCallback, or NULL) gets the result where the session is scheduled.
DA_HIDDEN void _DASessionRequest(DASessionRef session, const char* op, const char* name, uint64_t options,
	void* function, void* context);

// Has function (as above) called with that result where the session is scheduled.
DA_HIDDEN void _DASessionComplete(DASessionRef session, const char* name, void* function, void* context,
	DAReturn status, CFStringRef string);

#endif
