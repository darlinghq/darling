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

#include "DAPrivate.h"
#include "DAProtocol.h"

#include <string.h>

xpc_object_t _DAClientRequest(DASessionRef session, xpc_object_t message)
{
	if (session == NULL || session->connection == NULL)
		return NULL;

	xpc_object_t reply = xpc_connection_send_message_with_reply_sync(session->connection, message);
	if (reply != NULL && xpc_get_type(reply) != XPC_TYPE_DICTIONARY)
	{
		xpc_release(reply);
		return NULL;
	}
	return reply;
}

// The daemon sends the volume path as a path and UUIDs as strings (see DAProtocol.h).
static void restoreValueTypes(CFMutableDictionaryRef description)
{
	CFTypeRef path = CFDictionaryGetValue(description, kDADiskDescriptionVolumePathKey);
	if (path != NULL && CFGetTypeID(path) == CFStringGetTypeID())
	{
		CFURLRef url = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, true);
		if (url != NULL)
		{
			CFDictionarySetValue(description, kDADiskDescriptionVolumePathKey, url);
			CFRelease(url);
		}
	}

	const CFStringRef uuidKeys[] = { kDADiskDescriptionVolumeUUIDKey, kDADiskDescriptionMediaUUIDKey };
	for (size_t i = 0; i < sizeof(uuidKeys) / sizeof(uuidKeys[0]); i++)
	{
		CFTypeRef string = CFDictionaryGetValue(description, uuidKeys[i]);
		if (string == NULL || CFGetTypeID(string) != CFStringGetTypeID())
			continue;
		CFUUIDRef uuid = CFUUIDCreateFromString(NULL, string);
		if (uuid != NULL)
		{
			CFDictionarySetValue(description, uuidKeys[i], uuid);
			CFRelease(uuid);
		}
	}
}

CFMutableDictionaryRef _DAClientDescriptionFromMessage(xpc_object_t message)
{
	size_t length;
	const void* bytes = xpc_dictionary_get_data(message, DA_KEY_DESCRIPTION, &length);
	CFMutableDictionaryRef description = NULL;
	if (bytes != NULL)
	{
		CFDataRef data = CFDataCreate(NULL, bytes, length);
		CFPropertyListRef list = CFPropertyListCreateWithData(NULL, data, kCFPropertyListMutableContainers, NULL, NULL);
		CFRelease(data);
		if (list != NULL && CFGetTypeID(list) == CFDictionaryGetTypeID())
		{
			description = (CFMutableDictionaryRef) list;
			restoreValueTypes(description);
		}
		else if (list != NULL)
		{
			CFRelease(list);
		}
	}
	return description;
}

CFDictionaryRef _DAClientCopyDescription(DASessionRef session, const char* name)
{
	xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(message, DA_KEY_OP, DA_OP_DESCRIBE);
	xpc_dictionary_set_string(message, DA_KEY_NAME, name);
	xpc_object_t reply = _DAClientRequest(session, message);
	xpc_release(message);
	if (reply == NULL)
		return NULL;

	CFDictionaryRef description = _DAClientDescriptionFromMessage(reply);
	xpc_release(reply);
	return description;
}

bool _DAClientLookupPath(DASessionRef session, const char* path, char* name, size_t size)
{
	xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(message, DA_KEY_OP, DA_OP_LOOKUP_PATH);
	xpc_dictionary_set_string(message, DA_KEY_PATH, path);
	xpc_object_t reply = _DAClientRequest(session, message);
	xpc_release(message);
	if (reply == NULL)
		return false;

	const char* found = xpc_dictionary_get_string(reply, DA_KEY_NAME);
	bool ok = found != NULL && strlcpy(name, found, size) < size;
	xpc_release(reply);
	return ok;
}
