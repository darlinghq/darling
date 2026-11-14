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

#include "usernoted_dbus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_TIMEOUT_MS (10 * 1000)
#define MAX_ADDRESS_LENGTH 1024

#define NOTIFICATIONS_NAME "org.freedesktop.Notifications"
#define NOTIFICATIONS_PATH "/org/freedesktop/Notifications"
#define NOTIFICATIONS_INTERFACE "org.freedesktop.Notifications"

#define PORTAL_NAME "org.freedesktop.portal.Desktop"
#define PORTAL_PATH "/org/freedesktop/portal/desktop"
#define PORTAL_INTERFACE "org.freedesktop.portal.Notification"

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

char* _UNHostDBusSessionAddress(const struct usernoted_dbus* api)
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

DBusConnection* _UNHostDBusConnect(const struct usernoted_dbus* api)
{
	DBusConnection* conn;
	DBusError error;
	char* address = _UNHostDBusSessionAddress(api);

	if (address == NULL)
		return NULL;

	api->error_init(&error);
	conn = api->connection_open_private(address, &error);
	free(address);

	if (conn == NULL)
	{
		api->error_free(&error);
		return NULL;
	}

	if (!api->bus_register(conn, &error))
	{
		api->connection_close(conn);
		api->connection_unref(conn);
		api->error_free(&error);
		return NULL;
	}

	/* Subscribe to signals */
	api->bus_add_match(conn, "type='signal',interface='org.freedesktop.Notifications',member='ActionInvoked'", &error);
	api->bus_add_match(conn, "type='signal',interface='org.freedesktop.Notifications',member='NotificationClosed'", &error);
	api->bus_add_match(conn, "type='signal',interface='org.freedesktop.portal.Notification',member='ActionInvoked'", &error);

	api->error_free(&error);
	return conn;
}

void _UNHostDBusDisconnect(const struct usernoted_dbus* api, DBusConnection* conn)
{
	if (conn != NULL)
	{
		api->connection_close(conn);
		api->connection_unref(conn);
	}
}

static dbus_bool_t append_hint_byte(const struct usernoted_dbus* api, DBusMessageIter* dict, const char* key, unsigned char val)
{
	DBusMessageIter entry, variant;
	if (!api->message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry))
		return FALSE;
	if (!api->message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key))
		return FALSE;
	if (!api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "y", &variant))
		return FALSE;
	if (!api->message_iter_append_basic(&variant, DBUS_TYPE_BYTE, &val))
		return FALSE;
	if (!api->message_iter_close_container(&entry, &variant))
		return FALSE;
	return api->message_iter_close_container(dict, &entry);
}

static dbus_bool_t append_hint_string(const struct usernoted_dbus* api, DBusMessageIter* dict, const char* key, const char* val)
{
	DBusMessageIter entry, variant;
	if (!api->message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry))
		return FALSE;
	if (!api->message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key))
		return FALSE;
	if (!api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant))
		return FALSE;
	if (!api->message_iter_append_basic(&variant, DBUS_TYPE_STRING, &val))
		return FALSE;
	if (!api->message_iter_close_container(&entry, &variant))
		return FALSE;
	return api->message_iter_close_container(dict, &entry);
}

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
	int expire_timeout_ms)
{
	DBusMessage* msg;
	DBusMessage* reply;
	DBusMessageIter args, act_iter, hints_iter;
	DBusError error;
	const char* empty = "";
	const char* aname = app_name ? app_name : empty;
	const char* aicon = app_icon ? app_icon : empty;
	const char* summ = summary ? summary : empty;
	const char* btext = body ? body : empty;
	dbus_uint32_t rep_id = replaces_id;
	dbus_int32_t timeout = expire_timeout_ms >= 0 ? expire_timeout_ms : -1;
	uint32_t returned_id = 0;
	size_t i;
	int has_default_action = 0;

	if (conn == NULL)
		return 0;

	msg = api->message_new_method_call(NOTIFICATIONS_NAME, NOTIFICATIONS_PATH, NOTIFICATIONS_INTERFACE, "Notify");
	if (msg == NULL)
		return 0;

	api->message_iter_init_append(msg, &args);

	if (!api->message_iter_append_basic(&args, DBUS_TYPE_STRING, &aname)
		|| !api->message_iter_append_basic(&args, DBUS_TYPE_UINT32, &rep_id)
		|| !api->message_iter_append_basic(&args, DBUS_TYPE_STRING, &aicon)
		|| !api->message_iter_append_basic(&args, DBUS_TYPE_STRING, &summ)
		|| !api->message_iter_append_basic(&args, DBUS_TYPE_STRING, &btext))
	{
		api->message_unref(msg);
		return 0;
	}

	/* Actions array: [id, label, id, label, ...] */
	if (!api->message_iter_open_container(&args, DBUS_TYPE_ARRAY, "s", &act_iter))
	{
		api->message_unref(msg);
		return 0;
	}

	/* Check if "default" action was provided */
	for (i = 0; i + 1 < action_count; i += 2)
	{
		if (strcmp(actions[i], "default") == 0)
		{
			has_default_action = 1;
			break;
		}
	}

	/* Add "default" action for the plain click if not present */
	if (!has_default_action)
	{
		const char* def_id = "default";
		const char* def_label = "Open";
		api->message_iter_append_basic(&act_iter, DBUS_TYPE_STRING, &def_id);
		api->message_iter_append_basic(&act_iter, DBUS_TYPE_STRING, &def_label);
	}

	for (i = 0; i < action_count; i++)
	{
		const char* act = actions[i] ? actions[i] : empty;
		api->message_iter_append_basic(&act_iter, DBUS_TYPE_STRING, &act);
	}
	api->message_iter_close_container(&args, &act_iter);

	/* Hints dictionary: a{sv} */
	if (!api->message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &hints_iter))
	{
		api->message_unref(msg);
		return 0;
	}

	/* Urgency hint */
	if (urgency >= 0 && urgency <= 2)
	{
		append_hint_byte(api, &hints_iter, "urgency", (unsigned char) urgency);
	}

	/* Image hint */
	if (image_path != NULL && image_path[0] != '\0')
	{
		append_hint_string(api, &hints_iter, "image-path", image_path);
	}

	/* Sound hint */
	if (sound_file != NULL && sound_file[0] != '\0')
	{
		append_hint_string(api, &hints_iter, "sound-file", sound_file);
	}

	api->message_iter_close_container(&args, &hints_iter);

	/* Timeout */
	api->message_iter_append_basic(&args, DBUS_TYPE_INT32, &timeout);

	api->error_init(&error);
	reply = api->connection_send_with_reply_and_block(conn, msg,
		api->timeout_ms > 0 ? api->timeout_ms : DEFAULT_TIMEOUT_MS, &error);
	api->message_unref(msg);

	if (reply != NULL)
	{
		DBusMessageIter r_iter;
		if (api->message_iter_init(reply, &r_iter) && api->message_iter_get_arg_type(&r_iter) == DBUS_TYPE_UINT32)
		{
			api->message_iter_get_basic(&r_iter, &returned_id);
		}
		api->message_unref(reply);
	}
	api->error_free(&error);

	return returned_id;
}

int _UNHostDBusCloseNotification(const struct usernoted_dbus* api, DBusConnection* conn, uint32_t id)
{
	DBusMessage* msg;
	DBusMessage* reply;
	DBusError error;
	dbus_uint32_t dbus_id = id;

	if (conn == NULL || id == 0)
		return -1;

	msg = api->message_new_method_call(NOTIFICATIONS_NAME, NOTIFICATIONS_PATH, NOTIFICATIONS_INTERFACE, "CloseNotification");
	if (msg == NULL)
		return -1;

	DBusMessageIter args;
	api->message_iter_init_append(msg, &args);
	api->message_iter_append_basic(&args, DBUS_TYPE_UINT32, &dbus_id);

	api->error_init(&error);
	reply = api->connection_send_with_reply_and_block(conn, msg,
		api->timeout_ms > 0 ? api->timeout_ms : DEFAULT_TIMEOUT_MS, &error);
	api->message_unref(msg);

	if (reply != NULL)
	{
		api->message_unref(reply);
		api->error_free(&error);
		return 0;
	}
	api->error_free(&error);
	return -1;
}

int _UNHostDBusIsPortalAvailable(const struct usernoted_dbus* api, DBusConnection* conn)
{
	DBusMessage* msg;
	DBusMessage* reply;
	DBusError error;

	if (conn == NULL)
		return 0;

	msg = api->message_new_method_call(PORTAL_NAME, PORTAL_PATH, "org.freedesktop.DBus.Peer", "Ping");
	if (msg == NULL)
		return 0;

	api->error_init(&error);
	reply = api->connection_send_with_reply_and_block(conn, msg, 1000, &error);
	api->message_unref(msg);

	if (reply != NULL)
	{
		api->message_unref(reply);
		api->error_free(&error);
		return 1;
	}
	api->error_free(&error);
	return 0;
}

int _UNHostDBusPortalAddNotification(const struct usernoted_dbus* api,
	DBusConnection* conn,
	const char* id,
	const char* title,
	const char* body,
	const char* app_icon,
	const char* const* actions,
	size_t action_count)
{
	DBusMessage* msg;
	DBusMessage* reply;
	DBusMessageIter args, notif_dict, entry, variant, buttons_arr;
	DBusError error;
	const char* empty = "";
	const char* nid = id ? id : "1";
	const char* ntitle = title ? title : empty;
	const char* nbody = body ? body : empty;
	const char* def_action = "default";
	size_t i;

	if (conn == NULL)
		return -1;

	msg = api->message_new_method_call(PORTAL_NAME, PORTAL_PATH, PORTAL_INTERFACE, "AddNotification");
	if (msg == NULL)
		return -1;

	api->message_iter_init_append(msg, &args);
	api->message_iter_append_basic(&args, DBUS_TYPE_STRING, &nid);

	/* notification dictionary a{sv} */
	if (!api->message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &notif_dict))
	{
		api->message_unref(msg);
		return -1;
	}

	append_hint_string(api, &notif_dict, "title", ntitle);
	append_hint_string(api, &notif_dict, "body", nbody);
	append_hint_string(api, &notif_dict, "default-action", def_action);

	if (app_icon != NULL && app_icon[0] != '\0')
		append_hint_string(api, &notif_dict, "icon", app_icon);

	/* Buttons array: aa{sv} */
	if (action_count > 0)
	{
		const char* buttons_key = "buttons";
		api->message_iter_open_container(&notif_dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
		api->message_iter_append_basic(&entry, DBUS_TYPE_STRING, &buttons_key);
		api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "aa{sv}", &variant);
		api->message_iter_open_container(&variant, DBUS_TYPE_ARRAY, "a{sv}", &buttons_arr);

		for (i = 0; i + 1 < action_count; i += 2)
		{
			DBusMessageIter btn_dict;
			api->message_iter_open_container(&buttons_arr, DBUS_TYPE_ARRAY, "{sv}", &btn_dict);
			append_hint_string(api, &btn_dict, "action", actions[i]);
			append_hint_string(api, &btn_dict, "label", actions[i + 1]);
			api->message_iter_close_container(&buttons_arr, &btn_dict);
		}

		api->message_iter_close_container(&variant, &buttons_arr);
		api->message_iter_close_container(&entry, &variant);
		api->message_iter_close_container(&notif_dict, &entry);
	}

	api->message_iter_close_container(&args, &notif_dict);

	api->error_init(&error);
	reply = api->connection_send_with_reply_and_block(conn, msg,
		api->timeout_ms > 0 ? api->timeout_ms : DEFAULT_TIMEOUT_MS, &error);
	api->message_unref(msg);

	if (reply != NULL)
	{
		api->message_unref(reply);
		api->error_free(&error);
		return 0;
	}
	api->error_free(&error);
	return -1;
}

int _UNHostDBusPortalRemoveNotification(const struct usernoted_dbus* api, DBusConnection* conn, const char* id)
{
	DBusMessage* msg;
	DBusMessage* reply;
	DBusMessageIter args;
	DBusError error;
	const char* nid = id ? id : "";

	if (conn == NULL)
		return -1;

	msg = api->message_new_method_call(PORTAL_NAME, PORTAL_PATH, PORTAL_INTERFACE, "RemoveNotification");
	if (msg == NULL)
		return -1;

	api->message_iter_init_append(msg, &args);
	api->message_iter_append_basic(&args, DBUS_TYPE_STRING, &nid);

	api->error_init(&error);
	reply = api->connection_send_with_reply_and_block(conn, msg,
		api->timeout_ms > 0 ? api->timeout_ms : DEFAULT_TIMEOUT_MS, &error);
	api->message_unref(msg);

	if (reply != NULL)
	{
		api->message_unref(reply);
		api->error_free(&error);
		return 0;
	}
	api->error_free(&error);
	return -1;
}

int _UNHostDBusProcessMessages(const struct usernoted_dbus* api,
	DBusConnection* conn,
	UNActionInvokedCallback action_cb,
	UNNotificationClosedCallback closed_cb,
	void* context)
{
	int processed = 0;
	DBusMessage* msg;

	if (conn == NULL)
		return 0;

	/* Read available messages from socket non-blocking */
	api->connection_read_write(conn, 0);

	while ((msg = api->connection_pop_message(conn)) != NULL)
	{
		if (api->message_is_signal(msg, NOTIFICATIONS_INTERFACE, "ActionInvoked"))
		{
			DBusMessageIter it;
			uint32_t id = 0;
			const char* action_key = NULL;

			if (api->message_iter_init(msg, &it) && api->message_iter_get_arg_type(&it) == DBUS_TYPE_UINT32)
			{
				api->message_iter_get_basic(&it, &id);
				if (api->message_iter_next(&it) && api->message_iter_get_arg_type(&it) == DBUS_TYPE_STRING)
				{
					api->message_iter_get_basic(&it, &action_key);
					if (action_cb != NULL && action_key != NULL)
					{
						action_cb(id, action_key, context);
					}
				}
			}
			processed++;
		}
		else if (api->message_is_signal(msg, NOTIFICATIONS_INTERFACE, "NotificationClosed"))
		{
			DBusMessageIter it;
			uint32_t id = 0;
			uint32_t reason = 0;

			if (api->message_iter_init(msg, &it) && api->message_iter_get_arg_type(&it) == DBUS_TYPE_UINT32)
			{
				api->message_iter_get_basic(&it, &id);
				if (api->message_iter_next(&it) && api->message_iter_get_arg_type(&it) == DBUS_TYPE_UINT32)
				{
					api->message_iter_get_basic(&it, &reason);
					if (closed_cb != NULL)
					{
						closed_cb(id, reason, context);
					}
				}
			}
			processed++;
		}
		else if (api->message_is_signal(msg, PORTAL_INTERFACE, "ActionInvoked"))
		{
			DBusMessageIter it;
			const char* id_str = NULL;
			const char* action = NULL;

			if (api->message_iter_init(msg, &it) && api->message_iter_get_arg_type(&it) == DBUS_TYPE_STRING)
			{
				api->message_iter_get_basic(&it, &id_str);
				if (api->message_iter_next(&it) && api->message_iter_get_arg_type(&it) == DBUS_TYPE_STRING)
				{
					api->message_iter_get_basic(&it, &action);
					if (action_cb != NULL && id_str != NULL && action != NULL)
					{
						uint32_t id = (uint32_t) strtoul(id_str, NULL, 10);
						action_cb(id, action, context);
					}
				}
			}
			processed++;
		}

		api->message_unref(msg);
	}

	return processed;
}
