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

/*
 * Hands a URL to the host desktop through the OpenURI interface of the XDG
 * desktop portal (org.freedesktop.portal.OpenURI), on the user's D-Bus
 * session bus. Nothing else is used: if there is no portal, the URL is not
 * opened.
 *
 * This part has no Darling or CoreFoundation dependencies. The libdbus-1
 * functions and getenv come from the table below, so it can also be built
 * natively on Linux against libdbus-1 for testing.
 */

#ifndef LS_HOST_OPEN_BUS_H_
#define LS_HOST_OPEN_BUS_H_

#include "LSHostOpenURL.h"
#include <dbus/dbus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ls_host_dbus
{
	/* libdbus-1 */
	DBusConnection* (*connection_open_private)(const char* address, DBusError* error);
	dbus_bool_t (*bus_register)(DBusConnection* connection, DBusError* error);
	void (*connection_close)(DBusConnection* connection);
	void (*connection_unref)(DBusConnection* connection);
	DBusMessage* (*message_new_method_call)(const char* destination, const char* path,
		const char* interface, const char* method);
	void (*message_iter_init_append)(DBusMessage* message, DBusMessageIter* iter);
	dbus_bool_t (*message_iter_append_basic)(DBusMessageIter* iter, int type, const void* value);
	dbus_bool_t (*message_iter_open_container)(DBusMessageIter* iter, int type,
		const char* contained_signature, DBusMessageIter* sub);
	dbus_bool_t (*message_iter_close_container)(DBusMessageIter* iter, DBusMessageIter* sub);
	DBusMessage* (*connection_send_with_reply_and_block)(DBusConnection* connection,
		DBusMessage* message, int timeout_milliseconds, DBusError* error);
	void (*message_unref)(DBusMessage* message);
	void (*error_init)(DBusError* error);
	void (*error_free)(DBusError* error);
	char* (*address_escape_value)(const char* value);
	void (*free)(void* memory);

	/* The caller's environment */
	char* (*getenv)(const char* name);

	/* Per-call timeout in milliseconds; 0 selects the default */
	int timeout_ms;
};

enum
{
	LS_HOST_OPEN_OK = 0,
	/* The URL may not be handed to the host (see LSHostOpenURL.h). */
	LS_HOST_OPEN_INVALID_URL = -1,
	/* No usable session bus, or no OpenURI portal on it. */
	LS_HOST_OPEN_NO_PORTAL = -2,
	/* The portal refused the URL, failed, or did not answer in time. */
	LS_HOST_OPEN_FAILED = -3,
};

/*
 * The session bus address: DBUS_SESSION_BUS_ADDRESS if it is a single plain
 * unix socket address, otherwise $XDG_RUNTIME_DIR/bus when that variable is
 * unset. Returns a string to release with free(), or NULL.
 */
char* _LSHostOpenSessionAddress(const struct ls_host_dbus* api);

/* Opens the URL through the portal and returns one of the values above. */
int _LSHostOpenURL(const struct ls_host_dbus* api, const char* url);

#ifdef __cplusplus
}
#endif

#endif
