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

#include "DAServer.h"

#include <DiskArbitration/DADissenter.h>
#include <string.h>
#include <xpc/xpc.h>

#include "../DAProtocol.h"

static dispatch_queue_t g_queue;
static const DAModelEvents g_events = { 0 };

const DAModelEvents* DAServerEvents(void)
{
	return &g_events;
}

static void replyStatus(xpc_object_t reply, DAReturn status)
{
	xpc_dictionary_set_int64(reply, DA_KEY_STATUS, status);
}

static void handleDescribe(xpc_object_t message, xpc_object_t reply)
{
	const char* name = xpc_dictionary_get_string(message, DA_KEY_NAME);
	bool whole;
	DAMedia* media;

	if (name == NULL)
	{
		replyStatus(reply, kDAReturnBadArgument);
		return;
	}

	media = DAModelFindName(name, &whole);
	if (media == NULL)
	{
		// the disk may belong to a volume mounted since the last rescan
		DAModelRescan(DAServerEvents());
		media = DAModelFindName(name, &whole);
	}
	if (media == NULL)
	{
		replyStatus(reply, kDAReturnNotFound);
		return;
	}

	CFDataRef data = CFPropertyListCreateData(NULL, whole ? media->whole : media->leaf,
		kCFPropertyListBinaryFormat_v1_0, 0, NULL);
	if (data == NULL)
	{
		replyStatus(reply, kDAReturnError);
		return;
	}
	xpc_dictionary_set_data(reply, DA_KEY_DESCRIPTION, CFDataGetBytePtr(data), CFDataGetLength(data));
	CFRelease(data);
}

static void handleLookupPath(xpc_object_t message, xpc_object_t reply)
{
	const char* path = xpc_dictionary_get_string(message, DA_KEY_PATH);
	DAMedia* media;
	char name[32];

	if (path == NULL)
	{
		replyStatus(reply, kDAReturnBadArgument);
		return;
	}

	media = DAModelFindMountPoint(path);
	if (media == NULL)
	{
		DAModelRescan(DAServerEvents());
		media = DAModelFindMountPoint(path);
	}
	if (media == NULL)
	{
		replyStatus(reply, kDAReturnNotFound);
		return;
	}

	DAModelName(media, false, name, sizeof(name));
	xpc_dictionary_set_string(reply, DA_KEY_NAME, name);
}

static void handleMessage(xpc_connection_t peer, xpc_object_t message)
{
	const char* op = xpc_dictionary_get_string(message, DA_KEY_OP);
	xpc_object_t reply = xpc_dictionary_create_reply(message);

	if (reply == NULL)
		return;

	if (op != NULL && strcmp(op, DA_OP_DESCRIBE) == 0)
		handleDescribe(message, reply);
	else if (op != NULL && strcmp(op, DA_OP_LOOKUP_PATH) == 0)
		handleLookupPath(message, reply);
	else
		replyStatus(reply, kDAReturnUnsupported);

	xpc_connection_send_message(peer, reply);
	xpc_release(reply);
}

static void acceptPeer(xpc_connection_t peer)
{
	xpc_connection_set_event_handler(peer, ^(xpc_object_t message) {
		if (xpc_get_type(message) != XPC_TYPE_DICTIONARY)
			return;
		xpc_retain(message);
		dispatch_async(g_queue, ^{
			handleMessage(peer, message);
			xpc_release(message);
		});
	});
	xpc_connection_resume(peer);
}

void DAServerStart(dispatch_queue_t queue)
{
	g_queue = queue;

	xpc_connection_t listener = xpc_connection_create_mach_service(DA_SERVICE_NAME, queue,
		XPC_CONNECTION_MACH_SERVICE_LISTENER);
	xpc_connection_set_event_handler(listener, ^(xpc_object_t peer) {
		if (xpc_get_type(peer) == XPC_TYPE_CONNECTION)
			acceptPeer((xpc_connection_t) peer);
	});
	xpc_connection_resume(listener);
}
