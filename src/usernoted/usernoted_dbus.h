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

#ifndef _USERNOTED_DBUS_H_
#define _USERNOTED_DBUS_H_

#include <dbus/dbus.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct usernoted_dbus
{
	/* libdbus-1 */
	DBusConnection* (*connection_open_private)(const char* address, DBusError* error);
	dbus_bool_t (*bus_register)(DBusConnection* connection, DBusError* error);
	void (*bus_add_match)(DBusConnection* connection, const char* rule, DBusError* error);
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

typedef void (*UNActionInvokedCallback)(uint32_t id, const char* action_key, void* context);
typedef void (*UNNotificationClosedCallback)(uint32_t id, uint32_t reason, void* context);

char* _UNHostDBusSessionAddress(const struct usernoted_dbus* api);
DBusConnection* _UNHostDBusConnect(const struct usernoted_dbus* api);
void _UNHostDBusDisconnect(const struct usernoted_dbus* api, DBusConnection* conn);

/*
 * Posts a notification via org.freedesktop.Notifications.
 * actions is an array of strings in pairs: [action_id, action_title, ...].
 * Returns notification ID (> 0) on success, or 0 on failure.
 */
uint32_t _UNHostDBusNotify(const struct usernoted_dbus* api,
	DBusConnection* conn,
	const char* app_name,
	uint32_t replaces_id,
	const char* app_icon,
	const char* summary,
	const char* body,
	const char* const* actions,
	size_t action_count,
	const char* image_path,
	const char* sound_file,
	int urgency,
	int expire_timeout_ms);

/* Closes/withdraws a notification. Returns 0 on success. */
int _UNHostDBusCloseNotification(const struct usernoted_dbus* api, DBusConnection* conn, uint32_t id);

/* Checks if a portal fallback should be used. */
int _UNHostDBusIsPortalAvailable(const struct usernoted_dbus* api, DBusConnection* conn);

/* Posts a notification via org.freedesktop.portal.Notification. Returns 0 on success. */
int _UNHostDBusPortalAddNotification(const struct usernoted_dbus* api,
	DBusConnection* conn,
	const char* id,
	const char* title,
	const char* body,
	const char* app_icon,
	const char* const* actions,
	size_t action_count);

/* Removes a portal notification. Returns 0 on success. */
int _UNHostDBusPortalRemoveNotification(const struct usernoted_dbus* api, DBusConnection* conn, const char* id);

/*
 * Reads queued D-Bus messages, calls action_cb when ActionInvoked is received
 * and closed_cb when NotificationClosed is received.
 * Returns the number of messages processed.
 */
int _UNHostDBusProcessMessages(const struct usernoted_dbus* api,
	DBusConnection* conn,
	UNActionInvokedCallback action_cb,
	UNNotificationClosedCallback closed_cb,
	void* context);

#ifdef __cplusplus
}
#endif

#endif
