/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

Based on work by lucannez64 (public repository lucannez64/darling-lol,
commit 8f62017), GPL-3.0.

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

#include "LSHostOpenBus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_TIMEOUT_MS (10 * 1000)
#define MAX_ADDRESS_LENGTH 1024

#define PORTAL_NAME "org.freedesktop.portal.Desktop"
#define PORTAL_PATH "/org/freedesktop/portal/desktop"
#define PORTAL_INTERFACE "org.freedesktop.portal.OpenURI"

static int api_complete(const struct ls_host_dbus* api)
{
	return api != NULL && api->connection_open_private && api->bus_register && api->connection_close
		&& api->connection_unref && api->message_new_method_call && api->message_iter_init_append
		&& api->message_iter_append_basic && api->message_iter_open_container
		&& api->message_iter_close_container && api->connection_send_with_reply_and_block
		&& api->message_unref && api->error_init && api->error_free && api->address_escape_value
		&& api->free && api->getenv;
}

/*
 * D-Bus separates alternative addresses with ';' and accepts "unixexec:path=...",
 * which runs a host program, and "autolaunch:", which may start a bus, so
 * "unix:path=/x;unixexec:path=/bin/sh,..." must not get through. Only one plain
 * socket address (a path or an abstract name) is taken from the environment.
 */
static int session_address_allowed(const char* address)
{
	if (strncmp(address, "unix:path=", 10) != 0 && strncmp(address, "unix:abstract=", 14) != 0)
		return 0;
	if (strlen(address) >= MAX_ADDRESS_LENGTH)
		return 0;
	for (const unsigned char* c = (const unsigned char*) address; *c != '\0'; c++)
	{
		if (*c <= ' ' || *c >= 0x7f || *c == ';')
			return 0;
	}
	return strstr(address, "unixexec") == NULL;
}

char* _LSHostOpenSessionAddress(const struct ls_host_dbus* api)
{
	const char* env = api->getenv("DBUS_SESSION_BUS_ADDRESS");
	const char* runtime_dir;
	char path[MAX_ADDRESS_LENGTH];
	char* escaped;
	char* address;
	size_t length;

	if (env != NULL && env[0] != '\0')
		return session_address_allowed(env) ? strdup(env) : NULL;

	runtime_dir = api->getenv("XDG_RUNTIME_DIR");
	if (runtime_dir == NULL || runtime_dir[0] != '/')
		return NULL;
	if (snprintf(path, sizeof(path), "%s/bus", runtime_dir) >= (int) sizeof(path))
		return NULL;

	escaped = api->address_escape_value(path);
	if (escaped == NULL)
		return NULL;
	length = strlen("unix:path=") + strlen(escaped) + 1;
	address = malloc(length);
	if (address != NULL)
		snprintf(address, length, "unix:path=%s", escaped);
	api->free(escaped);
	return address;
}

static DBusConnection* open_session_bus(const struct ls_host_dbus* api)
{
	DBusConnection* connection;
	DBusError error;
	char* address = _LSHostOpenSessionAddress(api);

	if (address == NULL)
		return NULL;

	api->error_init(&error);
	connection = api->connection_open_private(address, &error);
	free(address);
	if (connection != NULL && !api->bus_register(connection, &error))
	{
		api->connection_close(connection);
		api->connection_unref(connection);
		connection = NULL;
	}
	api->error_free(&error);
	return connection;
}

static DBusMessage* new_open_uri_call(const struct ls_host_dbus* api, const char* url)
{
	DBusMessage* message = api->message_new_method_call(PORTAL_NAME, PORTAL_PATH, PORTAL_INTERFACE, "OpenURI");
	DBusMessageIter args, options;
	const char* parent_window = "";

	if (message == NULL)
		return NULL;

	api->message_iter_init_append(message, &args);
	if (!api->message_iter_append_basic(&args, DBUS_TYPE_STRING, &parent_window)
		|| !api->message_iter_append_basic(&args, DBUS_TYPE_STRING, &url)
		|| !api->message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &options)
		|| !api->message_iter_close_container(&args, &options))
	{
		api->message_unref(message);
		return NULL;
	}
	return message;
}

/* True when the portal, or its OpenURI interface, is simply not there. */
static int portal_unavailable(const char* error_name)
{
	static const char* const names[] = {
		DBUS_ERROR_SERVICE_UNKNOWN,
		DBUS_ERROR_NAME_HAS_NO_OWNER,
		DBUS_ERROR_UNKNOWN_METHOD,
		DBUS_ERROR_UNKNOWN_OBJECT,
		DBUS_ERROR_UNKNOWN_INTERFACE,
	};

	if (error_name == NULL)
		return 0;
	for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++)
	{
		if (strcmp(error_name, names[i]) == 0)
			return 1;
	}
	return 0;
}

int _LSHostOpenURL(const struct ls_host_dbus* api, const char* url)
{
	DBusConnection* connection;
	DBusMessage* call;
	DBusMessage* reply;
	DBusError error;
	int result;

	if (_LSHostOpenValidateURL(url) < 0)
		return LS_HOST_OPEN_INVALID_URL;
	if (!api_complete(api))
		return LS_HOST_OPEN_NO_PORTAL;

	connection = open_session_bus(api);
	if (connection == NULL)
		return LS_HOST_OPEN_NO_PORTAL;

	call = new_open_uri_call(api, url);
	if (call == NULL)
	{
		api->connection_close(connection);
		api->connection_unref(connection);
		return LS_HOST_OPEN_FAILED;
	}

	api->error_init(&error);
	reply = api->connection_send_with_reply_and_block(connection, call,
		api->timeout_ms > 0 ? api->timeout_ms : DEFAULT_TIMEOUT_MS, &error);
	if (reply != NULL)
	{
		/* The reply is the portal's request handle: the URL is with the desktop now. */
		result = LS_HOST_OPEN_OK;
		api->message_unref(reply);
	}
	else if (portal_unavailable(error.name))
		result = LS_HOST_OPEN_NO_PORTAL;
	else
		result = LS_HOST_OPEN_FAILED;

	api->error_free(&error);
	api->message_unref(call);
	api->connection_close(connection);
	api->connection_unref(connection);
	return result;
}
