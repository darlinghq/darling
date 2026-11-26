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

/*
 * The libdbus-1 of the host is reachable from the prefix only through the
 * wrapper library that src/native generates (DARLING_DBUS_WRAPPER_PATH).
 * This loader opens that wrapper once per process and binds the libdbus
 * functions that Darling components use into one table.
 */

#ifndef DARLING_DBUS_H_
#define DARLING_DBUS_H_

#include <dbus/dbus.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DARLING_DBUS_WRAPPER_PATH "/usr/lib/native/libdbus.dylib"

/* X(name, return type, parameter list): the libdbus function dbus_<name>. */
#define DARLING_DBUS_FUNCTIONS(X) \
	X(connection_open_private, DBusConnection*, (const char*, DBusError*)) \
	X(bus_register, dbus_bool_t, (DBusConnection*, DBusError*)) \
	X(bus_add_match, void, (DBusConnection*, const char*, DBusError*)) \
	X(bus_request_name, int, (DBusConnection*, const char*, unsigned int, DBusError*)) \
	X(bus_release_name, int, (DBusConnection*, const char*, DBusError*)) \
	X(connection_close, void, (DBusConnection*)) \
	X(connection_unref, void, (DBusConnection*)) \
	X(connection_get_is_connected, dbus_bool_t, (DBusConnection*)) \
	X(connection_get_unix_fd, dbus_bool_t, (DBusConnection*, int*)) \
	X(connection_read_write, dbus_bool_t, (DBusConnection*, int)) \
	X(connection_pop_message, DBusMessage*, (DBusConnection*)) \
	X(connection_send, dbus_bool_t, (DBusConnection*, DBusMessage*, dbus_uint32_t*)) \
	X(connection_flush, void, (DBusConnection*)) \
	X(connection_send_with_reply_and_block, DBusMessage*, (DBusConnection*, DBusMessage*, int, DBusError*)) \
	X(message_new_method_call, DBusMessage*, (const char*, const char*, const char*, const char*)) \
	X(message_new_method_return, DBusMessage*, (DBusMessage*)) \
	X(message_new_error, DBusMessage*, (DBusMessage*, const char*, const char*)) \
	X(message_new_signal, DBusMessage*, (const char*, const char*, const char*)) \
	X(message_iter_init_append, void, (DBusMessage*, DBusMessageIter*)) \
	X(message_iter_append_basic, dbus_bool_t, (DBusMessageIter*, int, const void*)) \
	X(message_iter_open_container, dbus_bool_t, (DBusMessageIter*, int, const char*, DBusMessageIter*)) \
	X(message_iter_close_container, dbus_bool_t, (DBusMessageIter*, DBusMessageIter*)) \
	X(message_iter_init, dbus_bool_t, (DBusMessage*, DBusMessageIter*)) \
	X(message_iter_get_arg_type, int, (DBusMessageIter*)) \
	X(message_iter_get_basic, void, (DBusMessageIter*, void*)) \
	X(message_iter_recurse, void, (DBusMessageIter*, DBusMessageIter*)) \
	X(message_iter_next, dbus_bool_t, (DBusMessageIter*)) \
	X(message_is_signal, dbus_bool_t, (DBusMessage*, const char*, const char*)) \
	X(message_is_method_call, dbus_bool_t, (DBusMessage*, const char*, const char*)) \
	X(message_get_type, int, (DBusMessage*)) \
	X(message_get_path, const char*, (DBusMessage*)) \
	X(message_get_sender, const char*, (DBusMessage*)) \
	X(message_get_interface, const char*, (DBusMessage*)) \
	X(message_get_member, const char*, (DBusMessage*)) \
	X(message_get_signature, const char*, (DBusMessage*)) \
	X(message_unref, void, (DBusMessage*)) \
	X(error_init, void, (DBusError*)) \
	X(error_free, void, (DBusError*)) \
	X(address_escape_value, char*, (const char*)) \
	X(free, void, (void*))

struct darling_dbus
{
#define DARLING_DBUS_DECLARE(name, ret, params) ret (*name) params;
	DARLING_DBUS_FUNCTIONS(DARLING_DBUS_DECLARE)
#undef DARLING_DBUS_DECLARE
};

enum
{
	/*
	 * If the wrapper cannot be loaded, use the host's libdbus-1 directly
	 * (libdbus-1.so.3, then libdbus-1.so). This is for processes that run
	 * on the host, such as tests; inside the prefix the wrapper is used.
	 */
	DARLING_DBUS_HOST_FALLBACK = 1,
};

/*
 * The function table, loaded on first use and shared by all callers in the
 * process; safe to call from any thread. Returns NULL if the wrapper (or,
 * with DARLING_DBUS_HOST_FALLBACK, the host library) cannot be loaded or lacks
 * one of the functions. The table is read-only and stays valid for the life of
 * the process. A failure is final: it is not retried.
 */
const struct darling_dbus* darling_dbus_get(unsigned int flags);

/*
 * Loads the library at path (the wrapper path when NULL) into *table, without
 * the process-wide sharing. Returns 0, -1 if the library cannot be loaded or
 * -2 if a function is missing (the library is then closed again). Used by
 * darling_dbus_get and by tests.
 */
int darling_dbus_load(const char* path, unsigned int flags, struct darling_dbus* table);

#ifdef __cplusplus
}
#endif

#endif
