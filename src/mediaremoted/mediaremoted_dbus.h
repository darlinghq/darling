/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

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

#ifndef _MEDIAREMOTED_DBUS_H_
#define _MEDIAREMOTED_DBUS_H_

#include <dbus/dbus.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mediaremoted_dbus
{
	/* libdbus-1 */
	DBusConnection* (*connection_open_private)(const char* address, DBusError* error);
	dbus_bool_t (*bus_register)(DBusConnection* connection, DBusError* error);
	int (*bus_request_name)(DBusConnection* connection, const char* name, unsigned int flags, DBusError* error);
	int (*bus_release_name)(DBusConnection* connection, const char* name, DBusError* error);
	void (*connection_close)(DBusConnection* connection);
	void (*connection_unref)(DBusConnection* connection);
	dbus_bool_t (*connection_read_write)(DBusConnection* connection, int timeout_milliseconds);
	DBusMessage* (*connection_pop_message)(DBusConnection* connection);
	dbus_bool_t (*connection_send)(DBusConnection* connection, DBusMessage* message, dbus_uint32_t* serial);
	void (*connection_flush)(DBusConnection* connection);
	DBusMessage* (*connection_send_with_reply_and_block)(DBusConnection* connection,
		DBusMessage* message, int timeout_milliseconds, DBusError* error);
	DBusMessage* (*message_new_method_call)(const char* destination, const char* path,
		const char* interface, const char* method);
	DBusMessage* (*message_new_method_return)(DBusMessage* method_call);
	DBusMessage* (*message_new_error)(DBusMessage* reply_to, const char* error_name, const char* error_message);
	DBusMessage* (*message_new_signal)(const char* path, const char* interface, const char* name);
	void (*message_iter_init_append)(DBusMessage* message, DBusMessageIter* iter);
	dbus_bool_t (*message_iter_append_basic)(DBusMessageIter* iter, int type, const void* value);
	dbus_bool_t (*message_iter_open_container)(DBusMessageIter* iter, int type,
		const char* contained_signature, DBusMessageIter* sub);
	dbus_bool_t (*message_iter_close_container)(DBusMessageIter* iter, DBusMessageIter* sub);
	dbus_bool_t (*message_iter_init)(DBusMessage* message, DBusMessageIter* iter);
	int (*message_iter_get_arg_type)(DBusMessageIter* iter);
	void (*message_iter_get_basic)(DBusMessageIter* iter, void* value);
	void (*message_iter_recurse)(DBusMessageIter* iter, DBusMessageIter* sub);
	dbus_bool_t (*message_iter_next)(DBusMessageIter* iter);
	int (*message_get_type)(DBusMessage* message);
	dbus_bool_t (*message_is_signal)(DBusMessage* message, const char* interface, const char* signal_name);
	dbus_bool_t (*message_is_method_call)(DBusMessage* message, const char* interface, const char* method);
	const char* (*message_get_path)(DBusMessage* message);
	const char* (*message_get_interface)(DBusMessage* message);
	const char* (*message_get_member)(DBusMessage* message);
	const char* (*message_get_signature)(DBusMessage* message);
	void (*message_unref)(DBusMessage* message);
	void (*error_init)(DBusError* error);
	void (*error_free)(DBusError* error);
	char* (*address_escape_value)(const char* value);
	void (*free)(void* memory);

	/* Caller's environment */
	char* (*getenv)(const char* name);

	int timeout_ms;
};

struct mr_player_state
{
	const char* identity;
	const char* desktop_entry;
	const char* playback_status; // "Playing", "Paused", "Stopped"
	const char* track_id; // a D-Bus object path, MR_NO_TRACK_ID if there is no track
	double rate;
	int64_t position_us; // current calculated position in µs
	int64_t duration_us; // track length in µs

	const char* title;
	const char* artist;
	const char* album;
	const char* composer;
	const char* genre;
	const char* art_url;
	int32_t track_number;
	int32_t disc_number;

	bool can_play;
	bool can_pause;
	bool can_go_next;
	bool can_go_previous;
	bool can_seek;
	bool can_control;
};

typedef int (*MRCommandCallback)(const char* command, double position_or_offset, void* context);

char* _MRHostDBusSessionAddress(const struct mediaremoted_dbus* api);
DBusConnection* _MRHostDBusConnect(const struct mediaremoted_dbus* api);
void _MRHostDBusDisconnect(const struct mediaremoted_dbus* api, DBusConnection* conn);

int _MRHostDBusClaimName(const struct mediaremoted_dbus* api, DBusConnection* conn, const char* name);
int _MRHostDBusReleaseName(const struct mediaremoted_dbus* api, DBusConnection* conn, const char* name);

int _MRHostDBusEmitPropertiesChanged(const struct mediaremoted_dbus* api,
	DBusConnection* conn,
	const struct mr_player_state* state);

int _MRHostDBusEmitSeeked(const struct mediaremoted_dbus* api,
	DBusConnection* conn,
	int64_t position_us);

int _MRHostDBusProcessMessages(const struct mediaremoted_dbus* api,
	DBusConnection* conn,
	const struct mr_player_state* state,
	MRCommandCallback command_cb,
	void* context);

#ifdef __cplusplus
}
#endif

#endif
