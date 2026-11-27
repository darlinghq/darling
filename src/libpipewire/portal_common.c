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
 * Shared pieces of the xdg-desktop-portal clients. The D-Bus function table is
 * injected so the same code runs against the wrapped host library in the
 * prefix and against the host library in the unit tests.
 */

#ifndef _GNU_SOURCE
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif

#include "portal_common.h"

#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEFAULT_TIMEOUT_MS 10000
#define MAX_ADDRESS_LENGTH 1024

enum
{
	DARLING_PORTAL_RESULT_OK = 0,
	DARLING_PORTAL_RESULT_FAILED = -1,
	DARLING_PORTAL_RESULT_DENIED = -2,
	DARLING_PORTAL_RESULT_CANCELLED = -4,
};

int darling_portal_api_complete(const struct darling_portal_api* api)
{
	const struct darling_dbus* dbus;

	if (!api || !api->getenv)
		return 0;
	dbus = api->dbus;
	return dbus && dbus->connection_open_private && dbus->bus_register && dbus->bus_get_unique_name
		&& dbus->bus_add_match && dbus->connection_close && dbus->connection_unref
		&& dbus->connection_read_write && dbus->connection_pop_message
		&& dbus->connection_send_with_reply_and_block && dbus->message_new_method_call
		&& dbus->message_iter_init_append && dbus->message_iter_append_basic
		&& dbus->message_iter_open_container && dbus->message_iter_close_container
		&& dbus->message_iter_init && dbus->message_iter_get_arg_type
		&& dbus->message_iter_get_basic && dbus->message_iter_recurse
		&& dbus->message_iter_next && dbus->message_is_signal && dbus->message_get_path
		&& dbus->message_unref && dbus->error_init && dbus->error_free
		&& dbus->address_escape_value && dbus->free;
}

/* Only plain Unix socket addresses are accepted: alternative addresses can
 * ask libdbus to start arbitrary helper programs. */
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

static char* session_address(const struct darling_portal_api* api)
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

DBusConnection* darling_portal_open_session_bus(const struct darling_portal_api* api)
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

int darling_portal_unavailable(const char* name)
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

int darling_portal_timeout_ms(const struct darling_portal_api* api)
{
	return api->timeout_ms > 0 ? api->timeout_ms : DEFAULT_TIMEOUT_MS;
}

void darling_portal_add_request_match(const struct darling_portal_api* api,
	DBusConnection* connection, DBusError* error)
{
	api->dbus->bus_add_match(connection,
		"type='signal',sender='" DARLING_PORTAL_NAME "',interface='"
		DARLING_REQUEST_INTERFACE "',member='Response'", error);
}

void darling_portal_options_begin(const struct darling_portal_api* api,
	DBusMessageIter* args, DBusMessageIter* options)
{
	api->dbus->message_iter_open_container(args, DBUS_TYPE_ARRAY, "{sv}", options);
}

void darling_portal_options_end(const struct darling_portal_api* api,
	DBusMessageIter* args, DBusMessageIter* options)
{
	api->dbus->message_iter_close_container(args, options);
}

static void open_entry(const struct darling_portal_api* api,
	DBusMessageIter* options, DBusMessageIter* entry, const char* key)
{
	const struct darling_dbus* dbus = api->dbus;

	dbus->message_iter_open_container(options, DBUS_TYPE_DICT_ENTRY, NULL, entry);
	dbus->message_iter_append_basic(entry, DBUS_TYPE_STRING, &key);
}

static void close_entry(const struct darling_portal_api* api,
	DBusMessageIter* options, DBusMessageIter* entry)
{
	const struct darling_dbus* dbus = api->dbus;

	dbus->message_iter_close_container(options, entry);
}

void darling_portal_options_add_string(const struct darling_portal_api* api,
	DBusMessageIter* options, const char* key, const char* value)
{
	DBusMessageIter entry, variant;

	open_entry(api, options, &entry, key);
	api->dbus->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant);
	api->dbus->message_iter_append_basic(&variant, DBUS_TYPE_STRING, &value);
	api->dbus->message_iter_close_container(&entry, &variant);
	close_entry(api, options, &entry);
}

void darling_portal_options_add_uint32(const struct darling_portal_api* api,
	DBusMessageIter* options, const char* key, dbus_uint32_t value)
{
	DBusMessageIter entry, variant;

	open_entry(api, options, &entry, key);
	api->dbus->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "u", &variant);
	api->dbus->message_iter_append_basic(&variant, DBUS_TYPE_UINT32, &value);
	api->dbus->message_iter_close_container(&entry, &variant);
	close_entry(api, options, &entry);
}

void darling_portal_options_add_bool(const struct darling_portal_api* api,
	DBusMessageIter* options, const char* key, dbus_bool_t value)
{
	DBusMessageIter entry, variant;

	open_entry(api, options, &entry, key);
	api->dbus->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "b", &variant);
	api->dbus->message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &value);
	api->dbus->message_iter_close_container(&entry, &variant);
	close_entry(api, options, &entry);
}

void darling_portal_append_empty_options(const struct darling_portal_api* api,
	DBusMessageIter* args)
{
	DBusMessageIter options;

	darling_portal_options_begin(api, args, &options);
	darling_portal_options_end(api, args, &options);
}

void darling_portal_append_handle_token(const struct darling_portal_api* api,
	DBusMessageIter* args, const char* token)
{
	darling_portal_append_string_option(api, args, "handle_token", token);
}

void darling_portal_append_string_option(const struct darling_portal_api* api,
	DBusMessageIter* args, const char* key, const char* value)
{
	DBusMessageIter options;

	darling_portal_options_begin(api, args, &options);
	darling_portal_options_add_string(api, &options, key, value);
	darling_portal_options_end(api, args, &options);
}

void darling_portal_append_uint32_option(const struct darling_portal_api* api,
	DBusMessageIter* args, const char* key, dbus_uint32_t value)
{
	DBusMessageIter options;

	darling_portal_options_begin(api, args, &options);
	darling_portal_options_add_uint32(api, &options, key, value);
	darling_portal_options_end(api, args, &options);
}

void darling_portal_append_bool_option(const struct darling_portal_api* api,
	DBusMessageIter* args, const char* key, dbus_bool_t value)
{
	DBusMessageIter options;

	darling_portal_options_begin(api, args, &options);
	darling_portal_options_add_bool(api, &options, key, value);
	darling_portal_options_end(api, args, &options);
}

/* Handle tokens must not collide with another client on the same session bus.
 * The portal asks for a string in the portable filename character set. */
void darling_portal_make_token(const struct darling_portal_api* api,
	DBusConnection* connection, const char* prefix, char* token, size_t size)
{
	static atomic_uint counter = 1;
	const char* sender = api->dbus->bus_get_unique_name(connection);
	char clean_prefix[32];
	size_t i = 0, out = 0;

	if (sender)
	{
		for (; sender[i] != '\0' && out + 1 < sizeof(clean_prefix); i++)
		{
			const char c = sender[i];
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
				|| (c >= '0' && c <= '9') || c == '_' || c == '-')
				clean_prefix[out++] = c;
		}
	}
	if (out == 0)
	{
		memcpy(clean_prefix, "darling", 7);
		out = 7;
	}
	clean_prefix[out] = '\0';
	snprintf(token, size, "%s_%s_%u", clean_prefix, prefix,
		atomic_fetch_add(&counter, 1u));
}

int darling_portal_wait_response(const struct darling_portal_api* api,
	DBusConnection* connection, const char* handle,
	darling_portal_response_fn callback, void* user_data)
{
	const struct darling_dbus* dbus = api->dbus;
	time_t deadline = time(NULL) + (darling_portal_timeout_ms(api) + 999) / 1000;
	int result = DARLING_PORTAL_RESULT_FAILED;

	while (time(NULL) <= deadline)
	{
		DBusMessage* message;

		dbus->connection_read_write(connection, 0);
		while ((message = dbus->connection_pop_message(connection)) != NULL)
		{
			const char* path = dbus->message_get_path(message);
			if (path && dbus->message_is_signal(message, DARLING_REQUEST_INTERFACE, "Response")
				&& strcmp(path, handle) == 0)
			{
				DBusMessageIter args;
				dbus_uint32_t response = 2;
				if (dbus->message_iter_init(message, &args)
					&& dbus->message_iter_get_arg_type(&args) == DBUS_TYPE_UINT32)
				{
					dbus->message_iter_get_basic(&args, &response);
					if (response == 0)
						result = DARLING_PORTAL_RESULT_OK;
					else if (response == 1)
						result = DARLING_PORTAL_RESULT_CANCELLED;
					else
						result = DARLING_PORTAL_RESULT_DENIED;
					if (callback && result == DARLING_PORTAL_RESULT_OK)
						callback(&args, user_data);
				}
			}
			dbus->message_unref(message);
		}
		if (result != DARLING_PORTAL_RESULT_FAILED || time(NULL) > deadline)
			break;
		{
			struct timespec pause = { 0, 1000000 };
			nanosleep(&pause, NULL);
		}
	}
	return result;
}
