/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

/*
 * A small client of the documented org.freedesktop.portal.Camera interface.
 * It uses Darling's injected libdbus function table so the same code runs
 * against the wrapped host library inside the prefix and against the host's
 * library in tests.
 */

#ifndef _GNU_SOURCE
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif

#include "camera_portal.h"

#include <errno.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PORTAL_NAME "org.freedesktop.portal.Desktop"
#define PORTAL_PATH "/org/freedesktop/portal/desktop"
#define CAMERA_INTERFACE "org.freedesktop.portal.Camera"
#define REQUEST_INTERFACE "org.freedesktop.portal.Request"
#define PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"

#define DEFAULT_TIMEOUT_MS 10000
#define MAX_ADDRESS_LENGTH 1024

static atomic_uint token_counter = 1;

static int api_complete(const struct darling_camera_portal_api* api)
{
	const struct darling_dbus* dbus;

	if (!api || !api->getenv)
		return 0;
	dbus = api->dbus;
	return dbus && dbus->connection_open_private && dbus->bus_register && dbus->bus_get_unique_name
		&& dbus->bus_add_match && dbus->connection_close && dbus->connection_unref && dbus->connection_read_write
		&& dbus->connection_pop_message && dbus->connection_send_with_reply_and_block
		&& dbus->message_new_method_call && dbus->message_iter_init_append
		&& dbus->message_iter_append_basic && dbus->message_iter_open_container
		&& dbus->message_iter_close_container && dbus->message_iter_init
		&& dbus->message_iter_get_arg_type && dbus->message_iter_get_basic
		&& dbus->message_iter_recurse && dbus->message_iter_next
		&& dbus->message_is_signal && dbus->message_get_path && dbus->message_unref
		&& dbus->error_init && dbus->error_free && dbus->address_escape_value && dbus->free;
}

/* Only one plain socket address is accepted: alternative and autolaunch
 * addresses can start arbitrary host programs. */
static int address_allowed(const char* address)
{
	const unsigned char* c;

	if (!address || address[0] == '\0' || strlen(address) >= MAX_ADDRESS_LENGTH)
		return 0;
	if (strncmp(address, "unix:path=", 10) != 0 && strncmp(address, "unix:abstract=", 14) != 0)
		return 0;
	for (c = (const unsigned char*) address; *c != '\0'; c++)
	{
		if (*c <= ' ' || *c >= 0x7f || *c == ';')
			return 0;
	}
	return strstr(address, "unixexec") == NULL;
}

static char* session_address(const struct darling_camera_portal_api* api)
{
	const struct darling_dbus* dbus = api->dbus;
	const char* address = api->getenv("DBUS_SESSION_BUS_ADDRESS");
	char path[MAX_ADDRESS_LENGTH];
	char* escaped;
	char* result;
	size_t length;

	if (address && address[0] != '\0')
		return address_allowed(address) ? strdup(address) : NULL;

	address = api->getenv("XDG_RUNTIME_DIR");
	if (!address || address[0] != '/')
		return NULL;
	if (snprintf(path, sizeof(path), "%s/bus", address) >= (int) sizeof(path))
		return NULL;

	escaped = dbus->address_escape_value(path);
	if (!escaped)
		return NULL;
	length = strlen("unix:path=") + strlen(escaped) + 1;
	result = malloc(length);
	if (result)
		snprintf(result, length, "unix:path=%s", escaped);
	dbus->free(escaped);
	return result;
}

static DBusConnection* open_session_bus(const struct darling_camera_portal_api* api)
{
	const struct darling_dbus* dbus = api->dbus;
	DBusConnection* connection;
	DBusError error;
	char* address = session_address(api);

	if (!address)
		return NULL;

	dbus->error_init(&error);
	connection = dbus->connection_open_private(address, &error);
	free(address);
	if (connection && !dbus->bus_register(connection, &error))
	{
		dbus->connection_close(connection);
		dbus->connection_unref(connection);
		connection = NULL;
	}
	dbus->error_free(&error);
	return connection;
}

static int portal_unavailable(const char* name)
{
	static const char* const names[] = {
		DBUS_ERROR_SERVICE_UNKNOWN,
		DBUS_ERROR_NAME_HAS_NO_OWNER,
		DBUS_ERROR_UNKNOWN_METHOD,
		DBUS_ERROR_UNKNOWN_OBJECT,
		DBUS_ERROR_UNKNOWN_INTERFACE,
		"org.freedesktop.DBus.Error.UnknownProperty",
	};
	size_t i;

	if (!name)
		return 0;
	for (i = 0; i < sizeof(names) / sizeof(names[0]); i++)
		if (strcmp(name, names[i]) == 0)
			return 1;
	return 0;
}

static int timeout_ms(const struct darling_camera_portal_api* api)
{
	return api->timeout_ms > 0 ? api->timeout_ms : DEFAULT_TIMEOUT_MS;
}

static void append_empty_options(const struct darling_camera_portal_api* api,
		DBusMessageIter* args)
{
	DBusMessageIter options;
	api->dbus->message_iter_open_container(args, DBUS_TYPE_ARRAY, "{sv}", &options);
	api->dbus->message_iter_close_container(args, &options);
}

static void append_handle_token(const struct darling_camera_portal_api* api,
		DBusMessageIter* args, const char* token)
{
	const struct darling_dbus* dbus = api->dbus;
	DBusMessageIter options, entry, variant;
	const char* key = "handle_token";

	dbus->message_iter_open_container(args, DBUS_TYPE_ARRAY, "{sv}", &options);
	dbus->message_iter_open_container(&options, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
	dbus->message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
	dbus->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant);
	dbus->message_iter_append_basic(&variant, DBUS_TYPE_STRING, &token);
	dbus->message_iter_close_container(&entry, &variant);
	dbus->message_iter_close_container(&options, &entry);
	dbus->message_iter_close_container(args, &options);
}

/* Handle tokens must not collide with another client on the same session bus.
 * The portal asks for a string in the portable filename character set. */
static void make_token(const struct darling_camera_portal_api* api,
		DBusConnection* connection, char* token, size_t size)
{
	const char* sender = api->dbus->bus_get_unique_name(connection);
	char prefix[32];
	size_t i = 0, out = 0;

	if (sender)
	{
		for (; sender[i] != '\0' && out + 1 < sizeof(prefix); i++)
		{
			const char c = sender[i];
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
				|| (c >= '0' && c <= '9') || c == '_' || c == '-')
				prefix[out++] = c;
		}
	}
	if (out == 0)
	{
		memcpy(prefix, "darling", 7);
		out = 7;
	}
	prefix[out] = '\0';
	snprintf(token, size, "%s_camera_%u", prefix, atomic_fetch_add(&token_counter, 1u));
}

static int bool_reply(const struct darling_camera_portal_api* api, DBusMessage* reply)
{
	const struct darling_dbus* dbus = api->dbus;
	DBusMessageIter args, variant;
	dbus_bool_t value = 0;

	if (!dbus->message_iter_init(reply, &args))
		return -1;
	if (dbus->message_iter_get_arg_type(&args) == DBUS_TYPE_VARIANT)
	{
		dbus->message_iter_recurse(&args, &variant);
		dbus->message_iter_get_basic(&variant, &value);
		return value != 0;
	}
	return -1;
}

int darling_camera_portal_is_present(const struct darling_camera_portal_api* api)
{
	const struct darling_dbus* dbus;
	DBusConnection* connection;
	DBusMessage* call;
	DBusMessage* reply;
	DBusError error;
	const char* interface = CAMERA_INTERFACE;
	const char* property = "IsCameraPresent";
	int present, result;

	if (!api_complete(api))
		return DARLING_CAMERA_PORTAL_NOT_AVAILABLE;
	dbus = api->dbus;
	connection = open_session_bus(api);
	if (!connection)
		return DARLING_CAMERA_PORTAL_NOT_AVAILABLE;

	call = dbus->message_new_method_call(PORTAL_NAME, PORTAL_PATH,
		PROPERTIES_INTERFACE, "Get");
	if (!call)
	{
		dbus->connection_close(connection);
		dbus->connection_unref(connection);
		return DARLING_CAMERA_PORTAL_FAILED;
	}

	DBusMessageIter args;
	dbus->message_iter_init_append(call, &args);
	if (!dbus->message_iter_append_basic(&args, DBUS_TYPE_STRING, &interface)
		|| !dbus->message_iter_append_basic(&args, DBUS_TYPE_STRING, &property))
	{
		dbus->message_unref(call);
		dbus->connection_close(connection);
		dbus->connection_unref(connection);
		return DARLING_CAMERA_PORTAL_FAILED;
	}

	dbus->error_init(&error);
	reply = dbus->connection_send_with_reply_and_block(connection, call,
		timeout_ms(api), &error);
	dbus->message_unref(call);

	if (reply)
	{
		present = bool_reply(api, reply);
		dbus->message_unref(reply);
		result = present < 0 ? DARLING_CAMERA_PORTAL_FAILED
			: (present ? DARLING_CAMERA_PORTAL_OK : DARLING_CAMERA_PORTAL_NOT_AVAILABLE);
	}
	else
		result = portal_unavailable(error.name)
			? DARLING_CAMERA_PORTAL_NOT_AVAILABLE : DARLING_CAMERA_PORTAL_FAILED;

	dbus->error_free(&error);
	dbus->connection_close(connection);
	dbus->connection_unref(connection);
	return result;
}

/* Waits for the Request.Response whose object path matches `handle`.
 * The match is installed for the whole interface, then filtered here. */
static int wait_for_response(const struct darling_camera_portal_api* api,
		DBusConnection* connection, const char* handle)
{
	const struct darling_dbus* dbus = api->dbus;
	time_t deadline = time(NULL) + (timeout_ms(api) + 999) / 1000;
	int result = DARLING_CAMERA_PORTAL_FAILED;

	while (time(NULL) <= deadline)
	{
		DBusMessage* message;

		dbus->connection_read_write(connection, 0);
		while ((message = dbus->connection_pop_message(connection)) != NULL)
		{
			const char* path = dbus->message_get_path(message);
			if (path && dbus->message_is_signal(message, REQUEST_INTERFACE, "Response")
				&& strcmp(path, handle) == 0)
			{
				DBusMessageIter args;
				dbus_uint32_t response = 2;
				if (dbus->message_iter_init(message, &args)
					&& dbus->message_iter_get_arg_type(&args) == DBUS_TYPE_UINT32)
				{
					dbus->message_iter_get_basic(&args, &response);
					if (response == 0)
						result = DARLING_CAMERA_PORTAL_OK;
					else if (response == 1)
						result = DARLING_CAMERA_PORTAL_CANCELLED;
					else
						result = DARLING_CAMERA_PORTAL_DENIED;
				}
			}
			dbus->message_unref(message);
		}
		if (result != DARLING_CAMERA_PORTAL_FAILED || time(NULL) > deadline)
			break;
		{
			struct timespec pause = { 0, 1000000 };
			nanosleep(&pause, NULL);
		}
	}
	return result;
}

static char* request_access(const struct darling_camera_portal_api* api,
		DBusConnection* connection, DBusError* error)
{
	const struct darling_dbus* dbus = api->dbus;
	char token[64];
	DBusMessage* call;
	DBusMessage* reply;
	DBusMessageIter args;
	const char* handle = NULL;
	char* copy;

	make_token(api, connection, token, sizeof(token));

	dbus->bus_add_match(connection,
		"type='signal',sender='" PORTAL_NAME "',interface='" REQUEST_INTERFACE "',member='Response'", error);
	if (error->name)
		return NULL;

	call = dbus->message_new_method_call(PORTAL_NAME, PORTAL_PATH,
		CAMERA_INTERFACE, "AccessCamera");
	if (!call)
		return NULL;

	dbus->message_iter_init_append(call, &args);
	append_handle_token(api, &args, token);
	reply = dbus->connection_send_with_reply_and_block(connection, call,
		timeout_ms(api), error);
	dbus->message_unref(call);
	if (!reply)
		return NULL;

	if (dbus->message_iter_init(reply, &args)
		&& dbus->message_iter_get_arg_type(&args) == DBUS_TYPE_OBJECT_PATH)
	{
		dbus->message_iter_get_basic(&args, &handle);
	}
	copy = handle ? strdup(handle) : NULL;
	dbus->message_unref(reply);
	return copy;
}

int darling_camera_portal_request_access(const struct darling_camera_portal_api* api)
{
	const struct darling_dbus* dbus;
	DBusConnection* connection;
	DBusError error;
	char* handle;
	int result;

	if (!api_complete(api))
		return DARLING_CAMERA_PORTAL_NOT_AVAILABLE;
	dbus = api->dbus;
	connection = open_session_bus(api);
	if (!connection)
		return DARLING_CAMERA_PORTAL_NOT_AVAILABLE;

	dbus->error_init(&error);
	handle = request_access(api, connection, &error);
	if (!handle)
		result = portal_unavailable(error.name)
			? DARLING_CAMERA_PORTAL_NOT_AVAILABLE : DARLING_CAMERA_PORTAL_FAILED;
	else
		result = wait_for_response(api, connection, handle);
	free(handle);
	dbus->error_free(&error);
	dbus->connection_close(connection);
	dbus->connection_unref(connection);
	return result;
}

int darling_camera_portal_open_remote(const struct darling_camera_portal_api* api, int* fd_out)
{
	const struct darling_dbus* dbus;
	DBusConnection* connection;
	DBusMessage* call;
	DBusMessage* reply;
	DBusMessageIter args;
	DBusError error;
	int unavailable;
	int fd = -1;

	if (!api_complete(api) || !fd_out)
		return DARLING_CAMERA_PORTAL_NOT_AVAILABLE;
	*fd_out = -1;
	dbus = api->dbus;
	connection = open_session_bus(api);
	if (!connection)
		return DARLING_CAMERA_PORTAL_NOT_AVAILABLE;

	call = dbus->message_new_method_call(PORTAL_NAME, PORTAL_PATH,
		CAMERA_INTERFACE, "OpenPipeWireRemote");
	if (!call)
	{
		dbus->connection_close(connection);
		dbus->connection_unref(connection);
		return DARLING_CAMERA_PORTAL_FAILED;
	}
	dbus->message_iter_init_append(call, &args);
	append_empty_options(api, &args);

	dbus->error_init(&error);
	reply = dbus->connection_send_with_reply_and_block(connection, call,
		timeout_ms(api), &error);
	dbus->message_unref(call);

	if (reply)
	{
		if (dbus->message_iter_init(reply, &args)
			&& dbus->message_iter_get_arg_type(&args) == DBUS_TYPE_UNIX_FD)
		{
			int received = -1;
			dbus->message_iter_get_basic(&args, &received);
			if (received >= 0)
			{
				/* libdbus hands us a new fd for the received message; the
				 * stream needs a close-on-exec copy and the original must
				 * not be leaked. */
				fd = fcntl(received, F_DUPFD_CLOEXEC, 0);
				close(received);
			}
		}
		dbus->message_unref(reply);
	}
	unavailable = portal_unavailable(error.name);
	dbus->error_free(&error);
	dbus->connection_close(connection);
	dbus->connection_unref(connection);

	if (fd < 0)
		return unavailable
			? DARLING_CAMERA_PORTAL_NOT_AVAILABLE : DARLING_CAMERA_PORTAL_FAILED;
	*fd_out = fd;
	return DARLING_CAMERA_PORTAL_OK;
}

const char* darling_camera_portal_strerror(int result)
{
	switch (result)
	{
		case DARLING_CAMERA_PORTAL_OK: return "success";
		case DARLING_CAMERA_PORTAL_NOT_AVAILABLE: return "camera portal unavailable";
		case DARLING_CAMERA_PORTAL_DENIED: return "camera access denied";
		case DARLING_CAMERA_PORTAL_CANCELLED: return "camera access request cancelled";
		case DARLING_CAMERA_PORTAL_FAILED: return "camera portal call failed";
		default: return "unknown camera portal error";
	}
}
