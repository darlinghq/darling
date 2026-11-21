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

#include "mediaremoted_dbus.h"
#include "mediaremoted_protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MPRIS_PATH "/org/mpris/MediaPlayer2"
#define MPRIS_INTERFACE "org.mpris.MediaPlayer2"
#define MPRIS_PLAYER_INTERFACE "org.mpris.MediaPlayer2.Player"
#define DBUS_PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"
#define DBUS_INTROSPECT_INTERFACE "org.freedesktop.DBus.Introspectable"

#define MAX_ADDRESS_LENGTH 1024

static const char* s_introspection_xml =
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
"<node>\n"
"  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
"    <method name=\"Introspect\">\n"
"      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
"  <interface name=\"org.freedesktop.DBus.Properties\">\n"
"    <method name=\"Get\">\n"
"      <arg name=\"interface_name\" direction=\"in\" type=\"s\"/>\n"
"      <arg name=\"property_name\" direction=\"in\" type=\"s\"/>\n"
"      <arg name=\"value\" direction=\"out\" type=\"v\"/>\n"
"    </method>\n"
"    <method name=\"Set\">\n"
"      <arg name=\"interface_name\" direction=\"in\" type=\"s\"/>\n"
"      <arg name=\"property_name\" direction=\"in\" type=\"s\"/>\n"
"      <arg name=\"value\" direction=\"in\" type=\"v\"/>\n"
"    </method>\n"
"    <method name=\"GetAll\">\n"
"      <arg name=\"interface_name\" direction=\"in\" type=\"s\"/>\n"
"      <arg name=\"properties\" direction=\"out\" type=\"a{sv}\"/>\n"
"    </method>\n"
"    <signal name=\"PropertiesChanged\">\n"
"      <arg name=\"interface_name\" type=\"s\"/>\n"
"      <arg name=\"changed_properties\" type=\"a{sv}\"/>\n"
"      <arg name=\"invalidated_properties\" type=\"as\"/>\n"
"    </signal>\n"
"  </interface>\n"
"  <interface name=\"org.mpris.MediaPlayer2\">\n"
"    <method name=\"Raise\"/>\n"
"    <method name=\"Quit\"/>\n"
"    <property name=\"CanQuit\" type=\"b\" access=\"read\"/>\n"
"    <property name=\"CanRaise\" type=\"b\" access=\"read\"/>\n"
"    <property name=\"HasTrackList\" type=\"b\" access=\"read\"/>\n"
"    <property name=\"Identity\" type=\"s\" access=\"read\"/>\n"
"    <property name=\"DesktopEntry\" type=\"s\" access=\"read\"/>\n"
"    <property name=\"SupportedUriSchemes\" type=\"as\" access=\"read\"/>\n"
"    <property name=\"SupportedMimeTypes\" type=\"as\" access=\"read\"/>\n"
"  </interface>\n"
"  <interface name=\"org.mpris.MediaPlayer2.Player\">\n"
"    <method name=\"Next\"/>\n"
"    <method name=\"Previous\"/>\n"
"    <method name=\"Pause\"/>\n"
"    <method name=\"PlayPause\"/>\n"
"    <method name=\"Stop\"/>\n"
"    <method name=\"Play\"/>\n"
"    <method name=\"Seek\">\n"
"      <arg name=\"Offset\" direction=\"in\" type=\"x\"/>\n"
"    </method>\n"
"    <method name=\"SetPosition\">\n"
"      <arg name=\"TrackId\" direction=\"in\" type=\"o\"/>\n"
"      <arg name=\"Position\" direction=\"in\" type=\"x\"/>\n"
"    </method>\n"
"    <method name=\"OpenUri\">\n"
"      <arg name=\"Uri\" direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <signal name=\"Seeked\">\n"
"      <arg name=\"Position\" type=\"x\"/>\n"
"    </signal>\n"
"    <property name=\"PlaybackStatus\" type=\"s\" access=\"read\"/>\n"
"    <property name=\"LoopStatus\" type=\"s\" access=\"readwrite\"/>\n"
"    <property name=\"Rate\" type=\"d\" access=\"readwrite\"/>\n"
"    <property name=\"Shuffle\" type=\"b\" access=\"readwrite\"/>\n"
"    <property name=\"Metadata\" type=\"a{sv}\" access=\"read\"/>\n"
"    <property name=\"Volume\" type=\"d\" access=\"readwrite\"/>\n"
"    <property name=\"Position\" type=\"x\" access=\"read\"/>\n"
"    <property name=\"MinimumRate\" type=\"d\" access=\"read\"/>\n"
"    <property name=\"MaximumRate\" type=\"d\" access=\"read\"/>\n"
"    <property name=\"CanGoNext\" type=\"b\" access=\"read\"/>\n"
"    <property name=\"CanGoPrevious\" type=\"b\" access=\"read\"/>\n"
"    <property name=\"CanPlay\" type=\"b\" access=\"read\"/>\n"
"    <property name=\"CanPause\" type=\"b\" access=\"read\"/>\n"
"    <property name=\"CanSeek\" type=\"b\" access=\"read\"/>\n"
"    <property name=\"CanControl\" type=\"b\" access=\"read\"/>\n"
"  </interface>\n"
"</node>\n";

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

char* _MRHostDBusSessionAddress(const struct mediaremoted_dbus* api)
{
	const char* custom = api->getenv(MEDIAREMOTED_DBUS_ADDRESS_ENV);
	if (custom != NULL && custom[0] != '\0')
		return session_address_allowed(custom) ? strdup(custom) : NULL;

	const char* env = api->getenv("DBUS_SESSION_BUS_ADDRESS");
	if (env != NULL && env[0] != '\0')
		return session_address_allowed(env) ? strdup(env) : NULL;

	const char* runtime_dir = api->getenv("XDG_RUNTIME_DIR");
	if (runtime_dir == NULL || runtime_dir[0] != '/')
		return NULL;

	char path[MAX_ADDRESS_LENGTH];
	if (snprintf(path, sizeof(path), "%s/bus", runtime_dir) >= (int) sizeof(path))
		return NULL;

	char* escaped = api->address_escape_value(path);
	if (escaped == NULL)
		return NULL;

	size_t length = strlen("unix:path=") + strlen(escaped) + 1;
	char* address = malloc(length);
	if (address != NULL)
		snprintf(address, length, "unix:path=%s", escaped);
	api->free(escaped);
	return address;
}

DBusConnection* _MRHostDBusConnect(const struct mediaremoted_dbus* api)
{
	DBusConnection* conn;
	DBusError error;
	char* address = _MRHostDBusSessionAddress(api);

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

	api->error_free(&error);
	return conn;
}

void _MRHostDBusDisconnect(const struct mediaremoted_dbus* api, DBusConnection* conn)
{
	if (conn != NULL)
	{
		api->connection_close(conn);
		api->connection_unref(conn);
	}
}

int _MRHostDBusClaimName(const struct mediaremoted_dbus* api, DBusConnection* conn, const char* name)
{
	if (conn == NULL || name == NULL || name[0] == '\0')
		return -1;

	DBusError error;
	api->error_init(&error);
	int res = api->bus_request_name(conn, name, DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE, &error);
	if (res == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER || res == DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER)
	{
		api->error_free(&error);
		return 0;
	}
	api->error_free(&error);
	return -1;
}

int _MRHostDBusReleaseName(const struct mediaremoted_dbus* api, DBusConnection* conn, const char* name)
{
	if (conn == NULL || name == NULL || name[0] == '\0')
		return -1;

	DBusError error;
	api->error_init(&error);
	int res = api->bus_release_name(conn, name, &error);
	api->error_free(&error);
	return (res == DBUS_RELEASE_NAME_REPLY_RELEASED) ? 0 : -1;
}

/* Helper methods for building D-Bus property dicts */
static dbus_bool_t append_variant_string(const struct mediaremoted_dbus* api, DBusMessageIter* dict, const char* key, const char* val)
{
	DBusMessageIter entry, variant;
	const char* v = val ? val : "";
	if (!api->message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry))
		return FALSE;
	if (!api->message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key))
		return FALSE;
	if (!api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant))
		return FALSE;
	if (!api->message_iter_append_basic(&variant, DBUS_TYPE_STRING, &v))
		return FALSE;
	if (!api->message_iter_close_container(&entry, &variant))
		return FALSE;
	return api->message_iter_close_container(dict, &entry);
}

static dbus_bool_t append_variant_bool(const struct mediaremoted_dbus* api, DBusMessageIter* dict, const char* key, dbus_bool_t val)
{
	DBusMessageIter entry, variant;
	if (!api->message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry))
		return FALSE;
	if (!api->message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key))
		return FALSE;
	if (!api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "b", &variant))
		return FALSE;
	if (!api->message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &val))
		return FALSE;
	if (!api->message_iter_close_container(&entry, &variant))
		return FALSE;
	return api->message_iter_close_container(dict, &entry);
}

static dbus_bool_t append_variant_double(const struct mediaremoted_dbus* api, DBusMessageIter* dict, const char* key, double val)
{
	DBusMessageIter entry, variant;
	if (!api->message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry))
		return FALSE;
	if (!api->message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key))
		return FALSE;
	if (!api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "d", &variant))
		return FALSE;
	if (!api->message_iter_append_basic(&variant, DBUS_TYPE_DOUBLE, &val))
		return FALSE;
	if (!api->message_iter_close_container(&entry, &variant))
		return FALSE;
	return api->message_iter_close_container(dict, &entry);
}

static dbus_bool_t append_variant_int64(const struct mediaremoted_dbus* api, DBusMessageIter* dict, const char* key, int64_t val)
{
	DBusMessageIter entry, variant;
	if (!api->message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry))
		return FALSE;
	if (!api->message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key))
		return FALSE;
	if (!api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "x", &variant))
		return FALSE;
	if (!api->message_iter_append_basic(&variant, DBUS_TYPE_INT64, &val))
		return FALSE;
	if (!api->message_iter_close_container(&entry, &variant))
		return FALSE;
	return api->message_iter_close_container(dict, &entry);
}

static dbus_bool_t append_variant_empty_str_array(const struct mediaremoted_dbus* api, DBusMessageIter* dict, const char* key)
{
	DBusMessageIter entry, variant, arr;
	if (!api->message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry))
		return FALSE;
	if (!api->message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key))
		return FALSE;
	if (!api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "as", &variant))
		return FALSE;
	if (!api->message_iter_open_container(&variant, DBUS_TYPE_ARRAY, "s", &arr))
		return FALSE;
	if (!api->message_iter_close_container(&variant, &arr))
		return FALSE;
	if (!api->message_iter_close_container(&entry, &variant))
		return FALSE;
	return api->message_iter_close_container(dict, &entry);
}

static dbus_bool_t append_metadata_string_list(const struct mediaremoted_dbus* api, DBusMessageIter* dict, const char* key, const char* val)
{
	DBusMessageIter entry, variant, arr;
	const char* v = val ? val : "";
	if (!api->message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry))
		return FALSE;
	if (!api->message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key))
		return FALSE;
	if (!api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "as", &variant))
		return FALSE;
	if (!api->message_iter_open_container(&variant, DBUS_TYPE_ARRAY, "s", &arr))
		return FALSE;
	if (val != NULL && val[0] != '\0')
	{
		if (!api->message_iter_append_basic(&arr, DBUS_TYPE_STRING, &v))
			return FALSE;
	}
	if (!api->message_iter_close_container(&variant, &arr))
		return FALSE;
	if (!api->message_iter_close_container(&entry, &variant))
		return FALSE;
	return api->message_iter_close_container(dict, &entry);
}

static dbus_bool_t append_metadata_dict(const struct mediaremoted_dbus* api, DBusMessageIter* parent, const struct mr_player_state* state)
{
	DBusMessageIter meta;
	if (!api->message_iter_open_container(parent, DBUS_TYPE_ARRAY, "{sv}", &meta))
		return FALSE;

	// mpris:trackid: must be a valid D-Bus object path
	const char* trackid_key = "mpris:trackid";
	const char* trackid_val = (state != NULL && (state->title != NULL || state->duration_us > 0)) ?
		"/org/mpris/MediaPlayer2/Track/1" : "/org/mpris/MediaPlayer2/TrackList/NoTrack";
	DBusMessageIter entry, variant;
	if (api->message_iter_open_container(&meta, DBUS_TYPE_DICT_ENTRY, NULL, &entry))
	{
		api->message_iter_append_basic(&entry, DBUS_TYPE_STRING, &trackid_key);
		if (api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "o", &variant))
		{
			api->message_iter_append_basic(&variant, DBUS_TYPE_OBJECT_PATH, &trackid_val);
			api->message_iter_close_container(&entry, &variant);
		}
		api->message_iter_close_container(&meta, &entry);
	}

	if (state != NULL)
	{
		if (state->duration_us > 0)
			append_variant_int64(api, &meta, "mpris:length", state->duration_us);
		if (state->title && state->title[0] != '\0')
			append_variant_string(api, &meta, "xesam:title", state->title);
		if (state->artist && state->artist[0] != '\0')
			append_metadata_string_list(api, &meta, "xesam:artist", state->artist);
		if (state->album && state->album[0] != '\0')
			append_variant_string(api, &meta, "xesam:album", state->album);
		if (state->composer && state->composer[0] != '\0')
			append_metadata_string_list(api, &meta, "xesam:composer", state->composer);
		if (state->genre && state->genre[0] != '\0')
			append_metadata_string_list(api, &meta, "xesam:genre", state->genre);
		if (state->art_url && state->art_url[0] != '\0')
			append_variant_string(api, &meta, "mpris:artUrl", state->art_url);
		if (state->track_number > 0)
		{
			int32_t tn = state->track_number;
			DBusMessageIter e, v;
			const char* k = "xesam:trackNumber";
			if (api->message_iter_open_container(&meta, DBUS_TYPE_DICT_ENTRY, NULL, &e))
			{
				api->message_iter_append_basic(&e, DBUS_TYPE_STRING, &k);
				if (api->message_iter_open_container(&e, DBUS_TYPE_VARIANT, "i", &v))
				{
					api->message_iter_append_basic(&v, DBUS_TYPE_INT32, &tn);
					api->message_iter_close_container(&e, &v);
				}
				api->message_iter_close_container(&meta, &e);
			}
		}
		if (state->disc_number > 0)
		{
			int32_t dn = state->disc_number;
			DBusMessageIter e, v;
			const char* k = "xesam:discNumber";
			if (api->message_iter_open_container(&meta, DBUS_TYPE_DICT_ENTRY, NULL, &e))
			{
				api->message_iter_append_basic(&e, DBUS_TYPE_STRING, &k);
				if (api->message_iter_open_container(&e, DBUS_TYPE_VARIANT, "i", &v))
				{
					api->message_iter_append_basic(&v, DBUS_TYPE_INT32, &dn);
					api->message_iter_close_container(&e, &v);
				}
				api->message_iter_close_container(&meta, &e);
			}
		}
	}

	return api->message_iter_close_container(parent, &meta);
}

int _MRHostDBusEmitPropertiesChanged(const struct mediaremoted_dbus* api,
	DBusConnection* conn,
	const struct mr_player_state* state)
{
	if (conn == NULL)
		return -1;

	DBusMessage* msg = api->message_new_signal(MPRIS_PATH, DBUS_PROPERTIES_INTERFACE, "PropertiesChanged");
	if (msg == NULL)
		return -1;

	DBusMessageIter it, dict, inv;
	const char* iface = MPRIS_PLAYER_INTERFACE;
	api->message_iter_init_append(msg, &it);
	api->message_iter_append_basic(&it, DBUS_TYPE_STRING, &iface);

	if (api->message_iter_open_container(&it, DBUS_TYPE_ARRAY, "{sv}", &dict))
	{
		const char* status = (state && state->playback_status) ? state->playback_status : "Stopped";
		append_variant_string(api, &dict, "PlaybackStatus", status);

		double rate = (state && state->rate > 0.0) ? state->rate : 1.0;
		append_variant_double(api, &dict, "Rate", rate);

		int64_t pos = state ? state->position_us : 0;
		append_variant_int64(api, &dict, "Position", pos);

		append_variant_bool(api, &dict, "CanPlay", (state && state->can_play) ? TRUE : FALSE);
		append_variant_bool(api, &dict, "CanPause", (state && state->can_pause) ? TRUE : FALSE);
		append_variant_bool(api, &dict, "CanGoNext", (state && state->can_go_next) ? TRUE : FALSE);
		append_variant_bool(api, &dict, "CanGoPrevious", (state && state->can_go_previous) ? TRUE : FALSE);
		append_variant_bool(api, &dict, "CanSeek", (state && state->can_seek) ? TRUE : FALSE);
		append_variant_bool(api, &dict, "CanControl", (state && state->can_control) ? TRUE : FALSE);

		// Metadata inside variant
		DBusMessageIter e, v;
		const char* meta_key = "Metadata";
		if (api->message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &e))
		{
			api->message_iter_append_basic(&e, DBUS_TYPE_STRING, &meta_key);
			if (api->message_iter_open_container(&e, DBUS_TYPE_VARIANT, "a{sv}", &v))
			{
				append_metadata_dict(api, &v, state);
				api->message_iter_close_container(&e, &v);
			}
			api->message_iter_close_container(&dict, &e);
		}

		api->message_iter_close_container(&it, &dict);
	}

	// Invalidated properties: empty array
	if (api->message_iter_open_container(&it, DBUS_TYPE_ARRAY, "as", &inv))
	{
		api->message_iter_close_container(&it, &inv);
	}

	api->connection_send(conn, msg, NULL);
	api->connection_flush(conn);
	api->message_unref(msg);
	return 0;
}

int _MRHostDBusEmitSeeked(const struct mediaremoted_dbus* api,
	DBusConnection* conn,
	int64_t position_us)
{
	if (conn == NULL)
		return -1;

	DBusMessage* msg = api->message_new_signal(MPRIS_PATH, MPRIS_PLAYER_INTERFACE, "Seeked");
	if (msg == NULL)
		return -1;

	DBusMessageIter it;
	api->message_iter_init_append(msg, &it);
	api->message_iter_append_basic(&it, DBUS_TYPE_INT64, &position_us);

	api->connection_send(conn, msg, NULL);
	api->connection_flush(conn);
	api->message_unref(msg);
	return 0;
}

static void handle_properties_get(const struct mediaremoted_dbus* api,
	DBusConnection* conn,
	DBusMessage* msg,
	const struct mr_player_state* state)
{
	DBusMessageIter it, sub, v;
	const char* iface = NULL;
	const char* prop = NULL;

	if (!api->message_iter_init(msg, &it) || api->message_iter_get_arg_type(&it) != DBUS_TYPE_STRING)
		return;
	api->message_iter_get_basic(&it, &iface);
	if (!api->message_iter_next(&it) || api->message_iter_get_arg_type(&it) != DBUS_TYPE_STRING)
		return;
	api->message_iter_get_basic(&it, &prop);

	DBusMessage* reply = api->message_new_method_return(msg);
	if (reply == NULL)
		return;

	api->message_iter_init_append(reply, &sub);

	if (strcmp(iface, MPRIS_INTERFACE) == 0)
	{
		if (strcmp(prop, "Identity") == 0)
		{
			const char* ident = (state && state->identity) ? state->identity : "Darling Media Player";
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "s", &v);
			api->message_iter_append_basic(&v, DBUS_TYPE_STRING, &ident);
			api->message_iter_close_container(&sub, &v);
		}
		else if (strcmp(prop, "DesktopEntry") == 0)
		{
			const char* de = (state && state->desktop_entry) ? state->desktop_entry : "";
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "s", &v);
			api->message_iter_append_basic(&v, DBUS_TYPE_STRING, &de);
			api->message_iter_close_container(&sub, &v);
		}
		else if (strcmp(prop, "CanQuit") == 0 || strcmp(prop, "CanRaise") == 0 || strcmp(prop, "HasTrackList") == 0)
		{
			dbus_bool_t b = FALSE;
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "b", &v);
			api->message_iter_append_basic(&v, DBUS_TYPE_BOOLEAN, &b);
			api->message_iter_close_container(&sub, &v);
		}
		else if (strcmp(prop, "SupportedUriSchemes") == 0 || strcmp(prop, "SupportedMimeTypes") == 0)
		{
			DBusMessageIter arr;
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "as", &v);
			api->message_iter_open_container(&v, DBUS_TYPE_ARRAY, "s", &arr);
			api->message_iter_close_container(&v, &arr);
			api->message_iter_close_container(&sub, &v);
		}
	}
	else if (strcmp(iface, MPRIS_PLAYER_INTERFACE) == 0)
	{
		if (strcmp(prop, "PlaybackStatus") == 0)
		{
			const char* ps = (state && state->playback_status) ? state->playback_status : "Stopped";
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "s", &v);
			api->message_iter_append_basic(&v, DBUS_TYPE_STRING, &ps);
			api->message_iter_close_container(&sub, &v);
		}
		else if (strcmp(prop, "Rate") == 0)
		{
			double r = (state && state->rate > 0.0) ? state->rate : 1.0;
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "d", &v);
			api->message_iter_append_basic(&v, DBUS_TYPE_DOUBLE, &r);
			api->message_iter_close_container(&sub, &v);
		}
		else if (strcmp(prop, "Position") == 0)
		{
			int64_t pos = state ? state->position_us : 0;
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "x", &v);
			api->message_iter_append_basic(&v, DBUS_TYPE_INT64, &pos);
			api->message_iter_close_container(&sub, &v);
		}
		else if (strcmp(prop, "Metadata") == 0)
		{
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "a{sv}", &v);
			append_metadata_dict(api, &v, state);
			api->message_iter_close_container(&sub, &v);
		}
		else if (strcmp(prop, "Volume") == 0 || strcmp(prop, "MinimumRate") == 0 || strcmp(prop, "MaximumRate") == 0)
		{
			double val = 1.0;
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "d", &v);
			api->message_iter_append_basic(&v, DBUS_TYPE_DOUBLE, &val);
			api->message_iter_close_container(&sub, &v);
		}
		else if (strcmp(prop, "CanControl") == 0)
		{
			dbus_bool_t b = (state && state->can_control) ? TRUE : FALSE;
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "b", &v);
			api->message_iter_append_basic(&v, DBUS_TYPE_BOOLEAN, &b);
			api->message_iter_close_container(&sub, &v);
		}
		else if (strcmp(prop, "CanPlay") == 0)
		{
			dbus_bool_t b = (state && state->can_play) ? TRUE : FALSE;
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "b", &v);
			api->message_iter_append_basic(&v, DBUS_TYPE_BOOLEAN, &b);
			api->message_iter_close_container(&sub, &v);
		}
		else if (strcmp(prop, "CanPause") == 0)
		{
			dbus_bool_t b = (state && state->can_pause) ? TRUE : FALSE;
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "b", &v);
			api->message_iter_append_basic(&v, DBUS_TYPE_BOOLEAN, &b);
			api->message_iter_close_container(&sub, &v);
		}
		else if (strcmp(prop, "CanGoNext") == 0)
		{
			dbus_bool_t b = (state && state->can_go_next) ? TRUE : FALSE;
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "b", &v);
			api->message_iter_append_basic(&v, DBUS_TYPE_BOOLEAN, &b);
			api->message_iter_close_container(&sub, &v);
		}
		else if (strcmp(prop, "CanGoPrevious") == 0)
		{
			dbus_bool_t b = (state && state->can_go_previous) ? TRUE : FALSE;
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "b", &v);
			api->message_iter_append_basic(&v, DBUS_TYPE_BOOLEAN, &b);
			api->message_iter_close_container(&sub, &v);
		}
		else if (strcmp(prop, "CanSeek") == 0)
		{
			dbus_bool_t b = (state && state->can_seek) ? TRUE : FALSE;
			api->message_iter_open_container(&sub, DBUS_TYPE_VARIANT, "b", &v);
			api->message_iter_append_basic(&v, DBUS_TYPE_BOOLEAN, &b);
			api->message_iter_close_container(&sub, &v);
		}
	}

	api->connection_send(conn, reply, NULL);
	api->connection_flush(conn);
	api->message_unref(reply);
}

static void handle_properties_get_all(const struct mediaremoted_dbus* api,
	DBusConnection* conn,
	DBusMessage* msg,
	const struct mr_player_state* state)
{
	DBusMessageIter it, sub, dict;
	const char* iface = NULL;

	if (api->message_iter_init(msg, &it) && api->message_iter_get_arg_type(&it) == DBUS_TYPE_STRING)
	{
		api->message_iter_get_basic(&it, &iface);
	}

	DBusMessage* reply = api->message_new_method_return(msg);
	if (reply == NULL)
		return;

	api->message_iter_init_append(reply, &sub);
	if (api->message_iter_open_container(&sub, DBUS_TYPE_ARRAY, "{sv}", &dict))
	{
		if (iface == NULL || strcmp(iface, MPRIS_INTERFACE) == 0)
		{
			const char* ident = (state && state->identity) ? state->identity : "Darling Media Player";
			const char* de = (state && state->desktop_entry) ? state->desktop_entry : "";
			append_variant_bool(api, &dict, "CanQuit", FALSE);
			append_variant_bool(api, &dict, "CanRaise", FALSE);
			append_variant_bool(api, &dict, "HasTrackList", FALSE);
			append_variant_string(api, &dict, "Identity", ident);
			append_variant_string(api, &dict, "DesktopEntry", de);
			append_variant_empty_str_array(api, &dict, "SupportedUriSchemes");
			append_variant_empty_str_array(api, &dict, "SupportedMimeTypes");
		}
		if (iface == NULL || strcmp(iface, MPRIS_PLAYER_INTERFACE) == 0)
		{
			const char* status = (state && state->playback_status) ? state->playback_status : "Stopped";
			append_variant_string(api, &dict, "PlaybackStatus", status);

			double rate = (state && state->rate > 0.0) ? state->rate : 1.0;
			append_variant_double(api, &dict, "Rate", rate);

			int64_t pos = state ? state->position_us : 0;
			append_variant_int64(api, &dict, "Position", pos);

			append_variant_double(api, &dict, "Volume", 1.0);
			append_variant_double(api, &dict, "MinimumRate", 1.0);
			append_variant_double(api, &dict, "MaximumRate", 1.0);

			append_variant_bool(api, &dict, "CanControl", (state && state->can_control) ? TRUE : FALSE);
			append_variant_bool(api, &dict, "CanPlay", (state && state->can_play) ? TRUE : FALSE);
			append_variant_bool(api, &dict, "CanPause", (state && state->can_pause) ? TRUE : FALSE);
			append_variant_bool(api, &dict, "CanGoNext", (state && state->can_go_next) ? TRUE : FALSE);
			append_variant_bool(api, &dict, "CanGoPrevious", (state && state->can_go_previous) ? TRUE : FALSE);
			append_variant_bool(api, &dict, "CanSeek", (state && state->can_seek) ? TRUE : FALSE);

			// Metadata inside variant
			DBusMessageIter e, v;
			const char* meta_key = "Metadata";
			if (api->message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &e))
			{
				api->message_iter_append_basic(&e, DBUS_TYPE_STRING, &meta_key);
				if (api->message_iter_open_container(&e, DBUS_TYPE_VARIANT, "a{sv}", &v))
				{
					append_metadata_dict(api, &v, state);
					api->message_iter_close_container(&e, &v);
				}
				api->message_iter_close_container(&dict, &e);
			}
		}
		api->message_iter_close_container(&sub, &dict);
	}

	api->connection_send(conn, reply, NULL);
	api->connection_flush(conn);
	api->message_unref(reply);
}

int _MRHostDBusProcessMessages(const struct mediaremoted_dbus* api,
	DBusConnection* conn,
	const struct mr_player_state* state,
	MRCommandCallback command_cb,
	void* context)
{
	int processed = 0;
	DBusMessage* msg;

	if (conn == NULL)
		return 0;

	api->connection_read_write(conn, 0);

	while ((msg = api->connection_pop_message(conn)) != NULL)
	{
		const char* path = api->message_get_path(msg);
		const char* iface = api->message_get_interface(msg);
		const char* member = api->message_get_member(msg);

		if (path != NULL && strcmp(path, MPRIS_PATH) == 0 && iface != NULL && member != NULL)
		{
			if (strcmp(iface, DBUS_INTROSPECT_INTERFACE) == 0 && strcmp(member, "Introspect") == 0)
			{
				DBusMessage* reply = api->message_new_method_return(msg);
				if (reply != NULL)
				{
					DBusMessageIter it;
					api->message_iter_init_append(reply, &it);
					api->message_iter_append_basic(&it, DBUS_TYPE_STRING, &s_introspection_xml);
					api->connection_send(conn, reply, NULL);
					api->connection_flush(conn);
					api->message_unref(reply);
				}
				processed++;
			}
			else if (strcmp(iface, DBUS_PROPERTIES_INTERFACE) == 0)
			{
				if (strcmp(member, "Get") == 0)
				{
					handle_properties_get(api, conn, msg, state);
					processed++;
				}
				else if (strcmp(member, "GetAll") == 0)
				{
					handle_properties_get_all(api, conn, msg, state);
					processed++;
				}
				else if (strcmp(member, "Set") == 0)
				{
					DBusMessage* reply = api->message_new_method_return(msg);
					if (reply != NULL)
					{
						api->connection_send(conn, reply, NULL);
						api->connection_flush(conn);
						api->message_unref(reply);
					}
					processed++;
				}
			}
			else if (strcmp(iface, MPRIS_INTERFACE) == 0)
			{
				if (strcmp(member, "Raise") == 0 || strcmp(member, "Quit") == 0)
				{
					DBusMessage* reply = api->message_new_method_return(msg);
					if (reply != NULL)
					{
						api->connection_send(conn, reply, NULL);
						api->connection_flush(conn);
						api->message_unref(reply);
					}
					processed++;
				}
				else
				{
					DBusMessage* reply = api->message_new_error(msg,
						"org.freedesktop.DBus.Error.UnknownMethod",
						"Method does not exist on org.mpris.MediaPlayer2");
					if (reply != NULL)
					{
						api->connection_send(conn, reply, NULL);
						api->connection_flush(conn);
						api->message_unref(reply);
					}
					processed++;
				}
			}
			else if (strcmp(iface, MPRIS_PLAYER_INTERFACE) == 0)
			{
				if (strcmp(member, "Play") == 0)
				{
					DBusMessage* reply = api->message_new_method_return(msg);
					if (reply != NULL)
					{
						api->connection_send(conn, reply, NULL);
						api->connection_flush(conn);
						api->message_unref(reply);
					}
					if (command_cb) command_cb("play", 0.0, context);
					processed++;
				}
				else if (strcmp(member, "Pause") == 0)
				{
					DBusMessage* reply = api->message_new_method_return(msg);
					if (reply != NULL)
					{
						api->connection_send(conn, reply, NULL);
						api->connection_flush(conn);
						api->message_unref(reply);
					}
					if (command_cb) command_cb("pause", 0.0, context);
					processed++;
				}
				else if (strcmp(member, "PlayPause") == 0)
				{
					DBusMessage* reply = api->message_new_method_return(msg);
					if (reply != NULL)
					{
						api->connection_send(conn, reply, NULL);
						api->connection_flush(conn);
						api->message_unref(reply);
					}
					if (command_cb) command_cb("togglePlayPause", 0.0, context);
					processed++;
				}
				else if (strcmp(member, "Stop") == 0)
				{
					DBusMessage* reply = api->message_new_method_return(msg);
					if (reply != NULL)
					{
						api->connection_send(conn, reply, NULL);
						api->connection_flush(conn);
						api->message_unref(reply);
					}
					if (command_cb) command_cb("stop", 0.0, context);
					processed++;
				}
				else if (strcmp(member, "Next") == 0)
				{
					DBusMessage* reply = api->message_new_method_return(msg);
					if (reply != NULL)
					{
						api->connection_send(conn, reply, NULL);
						api->connection_flush(conn);
						api->message_unref(reply);
					}
					if (command_cb) command_cb("nextTrack", 0.0, context);
					processed++;
				}
				else if (strcmp(member, "Previous") == 0)
				{
					DBusMessage* reply = api->message_new_method_return(msg);
					if (reply != NULL)
					{
						api->connection_send(conn, reply, NULL);
						api->connection_flush(conn);
						api->message_unref(reply);
					}
					if (command_cb) command_cb("previousTrack", 0.0, context);
					processed++;
				}
				else if (strcmp(member, "Seek") == 0)
				{
					DBusMessage* reply = api->message_new_method_return(msg);
					if (reply != NULL)
					{
						api->connection_send(conn, reply, NULL);
						api->connection_flush(conn);
						api->message_unref(reply);
					}
					DBusMessageIter it;
					int64_t offset_us = 0;
					if (api->message_iter_init(msg, &it) && api->message_iter_get_arg_type(&it) == DBUS_TYPE_INT64)
					{
						api->message_iter_get_basic(&it, &offset_us);
					}
					if (command_cb) command_cb("seek", (double)offset_us / 1000000.0, context);
					processed++;
				}
				else if (strcmp(member, "SetPosition") == 0)
				{
					DBusMessage* reply = api->message_new_method_return(msg);
					if (reply != NULL)
					{
						api->connection_send(conn, reply, NULL);
						api->connection_flush(conn);
						api->message_unref(reply);
					}
					DBusMessageIter it;
					const char* track_id = NULL;
					int64_t pos_us = 0;
					if (api->message_iter_init(msg, &it) && api->message_iter_get_arg_type(&it) == DBUS_TYPE_OBJECT_PATH)
					{
						api->message_iter_get_basic(&it, &track_id);
						if (api->message_iter_next(&it) && api->message_iter_get_arg_type(&it) == DBUS_TYPE_INT64)
						{
							api->message_iter_get_basic(&it, &pos_us);
						}
					}
					if (command_cb) command_cb("setPosition", (double)pos_us / 1000000.0, context);
					processed++;
				}
				else if (strcmp(member, "OpenUri") == 0)
				{
					DBusMessage* reply = api->message_new_method_return(msg);
					if (reply != NULL)
					{
						api->connection_send(conn, reply, NULL);
						api->connection_flush(conn);
						api->message_unref(reply);
					}
					processed++;
				}
				else
				{
					DBusMessage* reply = api->message_new_error(msg,
						"org.freedesktop.DBus.Error.UnknownMethod",
						"Method does not exist on org.mpris.MediaPlayer2.Player");
					if (reply != NULL)
					{
						api->connection_send(conn, reply, NULL);
						api->connection_flush(conn);
						api->message_unref(reply);
					}
					processed++;
				}
			}
			else
			{
				DBusMessage* reply = api->message_new_error(msg,
					"org.freedesktop.DBus.Error.UnknownInterface",
					"Interface not supported");
				if (reply != NULL)
				{
					api->connection_send(conn, reply, NULL);
					api->connection_flush(conn);
					api->message_unref(reply);
				}
				processed++;
			}
		}

		api->message_unref(msg);
	}

	return processed;
}
