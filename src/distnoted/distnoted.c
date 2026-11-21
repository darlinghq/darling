/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// The distributed notification daemon: routes the notifications posted to CoreFoundation's distributed center (and
// NSDistributedNotificationCenter) to the registrations of every connected process. launchd starts it on the first
// lookup of the service; the container has only the system domain, so it is a LaunchDaemon and every process of the
// container is a client. The messages are described in CoreFoundation/CFDistributedNotification_Private.h.

#include <CoreFoundation/CFDistributedNotification_Private.h>
#include <dispatch/dispatch.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <xpc/xpc.h>

typedef struct {
	int64_t id;
	char *name;     // NULL: any
	char *object;   // NULL: any
	int behavior;
} Registration;

typedef struct Client {
	xpc_connection_t connection;
	Registration *registrations;
	size_t count, capacity;
	bool suspended;
	struct Client *next;
} Client;

// Everything below runs on this serial queue.
static Client *gClients;

static void removeClient(Client *client) {
	for (Client **p = &gClients; *p != NULL; p = &(*p)->next) {
		if (*p == client) {
			*p = client->next;
			break;
		}
	}
	for (size_t i = 0; i < client->count; i++) {
		free(client->registrations[i].name);
		free(client->registrations[i].object);
	}
	free(client->registrations);
	xpc_release(client->connection);
	free(client);
}

// A string argument: absent is allowed (NULL), a longer or non-string one is not.
static bool optionalString(xpc_object_t message, const char *key, char **out) {
	xpc_object_t value = xpc_dictionary_get_value(message, key);
	*out = NULL;
	if (value == NULL) return true;
	if (xpc_get_type(value) != XPC_TYPE_STRING || xpc_string_get_length(value) > CFDN_MAX_STRING) return false;
	*out = strdup(xpc_string_get_string_ptr(value));
	return *out != NULL;
}

static void handleAdd(Client *client, xpc_object_t message) {
	xpc_object_t id = xpc_dictionary_get_value(message, CFDN_KEY_ID);
	xpc_object_t behavior = xpc_dictionary_get_value(message, CFDN_KEY_BEHAVIOR);
	if (id == NULL || xpc_get_type(id) != XPC_TYPE_INT64 || behavior == NULL || xpc_get_type(behavior) != XPC_TYPE_INT64) return;
	if (client->count >= CFDN_MAX_REGISTRATIONS) return;
	char *name, *object;
	if (!optionalString(message, CFDN_KEY_NAME, &name)) return;
	if (!optionalString(message, CFDN_KEY_OBJECT, &object)) {
		free(name);
		return;
	}
	if (client->count == client->capacity) {
		size_t capacity = client->capacity ? client->capacity * 2 : 8;
		Registration *grown = realloc(client->registrations, capacity * sizeof(Registration));
		if (grown == NULL) {
			free(name);
			free(object);
			return;
		}
		client->registrations = grown;
		client->capacity = capacity;
	}
	Registration *r = &client->registrations[client->count++];
	r->id = xpc_int64_get_value(id);
	r->name = name;
	r->object = object;
	r->behavior = (int)xpc_int64_get_value(behavior);
}

static void handleRemove(Client *client, xpc_object_t message) {
	xpc_object_t id = xpc_dictionary_get_value(message, CFDN_KEY_ID);
	if (id == NULL || xpc_get_type(id) != XPC_TYPE_INT64) return;
	int64_t wanted = xpc_int64_get_value(id);
	for (size_t i = 0; i < client->count; i++) {
		if (client->registrations[i].id != wanted) continue;
		free(client->registrations[i].name);
		free(client->registrations[i].object);
		client->registrations[i] = client->registrations[--client->count];
		return;
	}
}

static void handlePost(xpc_object_t message) {
	xpc_object_t nameValue = xpc_dictionary_get_value(message, CFDN_KEY_NAME);
	if (nameValue == NULL || xpc_get_type(nameValue) != XPC_TYPE_STRING || xpc_string_get_length(nameValue) > CFDN_MAX_STRING) return;
	const char *name = xpc_string_get_string_ptr(nameValue);
	const char *object = NULL;
	xpc_object_t objectValue = xpc_dictionary_get_value(message, CFDN_KEY_OBJECT);
	if (objectValue != NULL) {
		if (xpc_get_type(objectValue) != XPC_TYPE_STRING || xpc_string_get_length(objectValue) > CFDN_MAX_STRING) return;
		object = xpc_string_get_string_ptr(objectValue);
	}
	xpc_object_t userInfo = xpc_dictionary_get_value(message, CFDN_KEY_USERINFO);
	if (userInfo != NULL && (xpc_get_type(userInfo) != XPC_TYPE_DATA || xpc_data_get_length(userInfo) > CFDN_MAX_USERINFO)) return;
	bool immediate = xpc_dictionary_get_bool(message, CFDN_KEY_IMMEDIATE);

	for (Client *client = gClients; client != NULL; client = client->next) {
		xpc_object_t ids = NULL;
		for (size_t i = 0; i < client->count; i++) {
			Registration *r = &client->registrations[i];
			if (r->name != NULL && strcmp(r->name, name) != 0) continue;
			if (r->object != NULL && (object == NULL || strcmp(r->object, object) != 0)) continue;
			if (!CFDN_ShouldDeliver(client->suspended, immediate, r->behavior)) continue;
			if (ids == NULL) ids = xpc_array_create(NULL, 0);
			xpc_array_set_int64(ids, XPC_ARRAY_APPEND, r->id);
		}
		if (ids == NULL) continue;
		xpc_object_t delivery = xpc_dictionary_create(NULL, NULL, 0);
		xpc_dictionary_set_int64(delivery, CFDN_KEY_OP, CFDN_OP_DELIVER);
		xpc_dictionary_set_value(delivery, CFDN_KEY_IDS, ids);
		xpc_dictionary_set_string(delivery, CFDN_KEY_NAME, name);
		if (object != NULL) xpc_dictionary_set_string(delivery, CFDN_KEY_OBJECT, object);
		if (userInfo != NULL) xpc_dictionary_set_value(delivery, CFDN_KEY_USERINFO, userInfo);
		xpc_connection_send_message(client->connection, delivery);
		xpc_release(delivery);
		xpc_release(ids);
	}
}

static void handleMessage(Client *client, xpc_object_t message) {
	xpc_object_t opValue = xpc_dictionary_get_value(message, CFDN_KEY_OP);
	if (opValue == NULL || xpc_get_type(opValue) != XPC_TYPE_INT64) return;
	switch (xpc_int64_get_value(opValue)) {
	case CFDN_OP_HELLO: {
		xpc_object_t reply = xpc_dictionary_create_reply(message);
		if (reply != NULL) {
			xpc_connection_send_message(client->connection, reply);
			xpc_release(reply);
		}
		break;
	}
	case CFDN_OP_ADD:
		handleAdd(client, message);
		break;
	case CFDN_OP_REMOVE:
		handleRemove(client, message);
		break;
	case CFDN_OP_SUSPEND:
		client->suspended = xpc_dictionary_get_bool(message, CFDN_KEY_SUSPENDED);
		break;
	case CFDN_OP_POST:
		handlePost(message);
		break;
	default:
		break;
	}
}

int main(void) {
	dispatch_queue_t queue = dispatch_queue_create("com.apple.distnoted", DISPATCH_QUEUE_SERIAL);
	xpc_connection_t listener = xpc_connection_create_mach_service(CFDN_SERVICE_NAME, queue, XPC_CONNECTION_MACH_SERVICE_LISTENER);
	if (listener == NULL) return 1;
	xpc_connection_set_event_handler(listener, ^(xpc_object_t peer) {
		if (xpc_get_type(peer) != XPC_TYPE_CONNECTION) return;
		Client *client = calloc(1, sizeof(Client));
		if (client == NULL) return;
		client->connection = xpc_retain(peer);
		client->next = gClients;
		gClients = client;
		xpc_connection_set_target_queue(peer, queue);
		xpc_connection_set_event_handler(peer, ^(xpc_object_t event) {
			if (xpc_get_type(event) == XPC_TYPE_DICTIONARY) {
				handleMessage(client, event);
			} else if (event == XPC_ERROR_CONNECTION_INVALID || event == XPC_ERROR_CONNECTION_INTERRUPTED) {
				// A client that went away takes its registrations with it.
				removeClient(client);
			}
		});
		xpc_connection_resume(peer);
	});
	xpc_connection_resume(listener);
	dispatch_main();
}
