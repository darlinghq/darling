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
#include <stdlib.h>
#include <string.h>
#include <xpc/xpc.h>

#include "../DAProtocol.h"
#include "DAOperations.h"

static dispatch_queue_t g_queue;

// The connections that said hello, which get every event.
static xpc_connection_t* g_subscribers;
static size_t g_subscriberCount;

dispatch_queue_t DAServerQueue(void)
{
	return g_queue;
}

void DAServerForEachSubscriber(void (^block)(xpc_connection_t peer))
{
	for (size_t i = 0; i < g_subscriberCount; i++)
		block(g_subscribers[i]);
}

bool DAServerSetDescription(xpc_object_t message, CFDictionaryRef description)
{
	CFDataRef data = CFPropertyListCreateData(NULL, description, kCFPropertyListBinaryFormat_v1_0, 0, NULL);
	if (data == NULL)
		return false;
	xpc_dictionary_set_data(message, DA_KEY_DESCRIPTION, CFDataGetBytePtr(data), CFDataGetLength(data));
	CFRelease(data);
	return true;
}

static void broadcast(const char* event, const char* name, CFArrayRef keys, CFDictionaryRef description)
{
	if (g_subscriberCount == 0)
		return;

	xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(message, DA_KEY_EVENT, event);
	xpc_dictionary_set_string(message, DA_KEY_NAME, name);
	DAServerSetDescription(message, description);
	if (keys != NULL)
	{
		xpc_object_t array = xpc_array_create(NULL, 0);
		for (CFIndex i = 0; i < CFArrayGetCount(keys); i++)
		{
			char key[256];
			if (CFStringGetCString(CFArrayGetValueAtIndex(keys, i), key, sizeof(key), kCFStringEncodingUTF8))
				xpc_array_set_string(array, XPC_ARRAY_APPEND, key);
		}
		xpc_dictionary_set_value(message, DA_KEY_KEYS, array);
		xpc_release(array);
	}

	for (size_t i = 0; i < g_subscriberCount; i++)
		xpc_connection_send_message(g_subscribers[i], message);
	xpc_release(message);
}

static void appeared(void* context, const char* name, CFDictionaryRef description)
{
	broadcast(DA_EVENT_APPEARED, name, NULL, description);
}

static void disappeared(void* context, const char* name, CFDictionaryRef description)
{
	broadcast(DA_EVENT_DISAPPEARED, name, NULL, description);
}

static void changed(void* context, const char* name, CFArrayRef keys, CFDictionaryRef description)
{
	broadcast(DA_EVENT_CHANGED, name, keys, description);
}

static const DAModelEvents g_events = {
	.appeared = appeared,
	.disappeared = disappeared,
	.changed = changed,
};

const DAModelEvents* DAServerEvents(void)
{
	return &g_events;
}

static void subscribe(xpc_connection_t peer)
{
	for (size_t i = 0; i < g_subscriberCount; i++)
	{
		if (g_subscribers[i] == peer)
			return;
	}
	xpc_connection_t* grown = realloc(g_subscribers, sizeof(*grown) * (g_subscriberCount + 1));
	if (grown == NULL)
		return;
	g_subscribers = grown;
	g_subscribers[g_subscriberCount++] = xpc_retain(peer);
}

static void unsubscribe(xpc_connection_t peer)
{
	for (size_t i = 0; i < g_subscriberCount; i++)
	{
		if (g_subscribers[i] == peer)
		{
			xpc_release(peer);
			g_subscribers[i] = g_subscribers[--g_subscriberCount];
			return;
		}
	}
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

	if (!DAServerSetDescription(reply, whole ? media->whole : media->leaf))
		replyStatus(reply, kDAReturnError);
}

static void appendDisk(xpc_object_t disks, const DAMedia* media, bool whole)
{
	char name[32];
	xpc_object_t disk = xpc_dictionary_create(NULL, NULL, 0);

	DAModelName(media, whole, name, sizeof(name));
	xpc_dictionary_set_string(disk, DA_KEY_NAME, name);
	if (DAServerSetDescription(disk, whole ? media->whole : media->leaf))
		xpc_array_append_value(disks, disk);
	xpc_release(disk);
}

// The peer gets every event from now on; the reply lists the disks there are now.
static void handleHello(xpc_connection_t peer, xpc_object_t reply)
{
	xpc_object_t disks = xpc_array_create(NULL, 0);

	subscribe(peer);
	for (DAMedia* media = DAModelFirst(); media != NULL; media = media->next)
	{
		appendDisk(disks, media, false);
		appendDisk(disks, media, true);
	}
	xpc_dictionary_set_value(reply, DA_KEY_DISKS, disks);
	xpc_release(disks);
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
	else if (op != NULL && strcmp(op, DA_OP_HELLO) == 0)
		handleHello(peer, reply);
	else if (op != NULL && strcmp(op, DA_OP_UNMOUNT) == 0)
	{
		DAOperationUnmount(peer, message, reply); // replies when it is done
		xpc_release(reply);
		return;
	}
	else if (op != NULL && strcmp(op, DA_OP_EJECT) == 0)
	{
		DAOperationEject(peer, message, reply);
		xpc_release(reply);
		return;
	}
	else
		replyStatus(reply, kDAReturnUnsupported);

	xpc_connection_send_message(peer, reply);
	xpc_release(reply);
}

static void acceptPeer(xpc_connection_t peer)
{
	xpc_connection_set_event_handler(peer, ^(xpc_object_t message) {
		if (message == XPC_ERROR_CONNECTION_INVALID)
		{
			dispatch_async(g_queue, ^{
				unsubscribe(peer);
			});
			return;
		}
		if (xpc_get_type(message) != XPC_TYPE_DICTIONARY)
			return;
		xpc_retain(peer);
		xpc_retain(message);
		dispatch_async(g_queue, ^{
			handleMessage(peer, message);
			xpc_release(message);
			xpc_release(peer);
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
