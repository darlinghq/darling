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
 * A client of org.freedesktop.portal.ScreenCast. The portal owns the user
 * prompt and the PipeWire graph; this code only collects the session handle,
 * the selected streams and the remote fd.
 */

#ifndef _GNU_SOURCE
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif

#include "screencast_portal.h"
#include "portal_common.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SCREENCAST_INTERFACE "org.freedesktop.portal.ScreenCast"
#define SESSION_INTERFACE "org.freedesktop.portal.Session"

#define MAX_HANDLE_LENGTH 256
#define MAX_STREAMS 64

struct darling_screencast_session
{
	struct darling_portal_api api;
	struct darling_dbus dbus;
	DBusConnection* connection;
	char handle[MAX_HANDLE_LENGTH];
	int fd;
	struct darling_screencast_stream* streams;
	uint32_t stream_count;
};

struct stream_parse_context
{
	const struct darling_dbus* dbus;
	struct darling_screencast_stream* streams;
	uint32_t count;
	uint32_t source_types;
};

struct handle_context
{
	const struct darling_dbus* dbus;
	char handle[MAX_HANDLE_LENGTH];
};

typedef void (*append_options_fn)(const struct darling_portal_api* api,
	DBusConnection* connection, DBusMessageIter* args, const char* token, void* user_data);

static const struct darling_portal_api* portal_api(
	const struct darling_screencast_portal_api* api)
{
	return (const struct darling_portal_api*) api;
}

static int get_uint32_property(const struct darling_portal_api* api,
	DBusConnection* connection, const char* property, uint32_t* out)
{
	const struct darling_dbus* dbus = api->dbus;
	DBusMessage* call;
	DBusMessage* reply;
	DBusMessageIter args, variant;
	DBusError error;
	const char* interface = SCREENCAST_INTERFACE;
	dbus_uint32_t value = 0;
	int result = DARLING_SCREENCAST_FAILED;

	call = dbus->message_new_method_call(DARLING_PORTAL_NAME, DARLING_PORTAL_PATH,
		DARLING_PROPERTIES_INTERFACE, "Get");
	if (!call)
		return DARLING_SCREENCAST_FAILED;
	dbus->message_iter_init_append(call, &args);
	if (!dbus->message_iter_append_basic(&args, DBUS_TYPE_STRING, &interface)
		|| !dbus->message_iter_append_basic(&args, DBUS_TYPE_STRING, &property))
	{
		dbus->message_unref(call);
		return DARLING_SCREENCAST_FAILED;
	}

	dbus->error_init(&error);
	reply = dbus->connection_send_with_reply_and_block(connection, call,
		darling_portal_timeout_ms(api), &error);
	dbus->message_unref(call);

	if (reply)
	{
		if (dbus->message_iter_init(reply, &args)
			&& dbus->message_iter_get_arg_type(&args) == DBUS_TYPE_VARIANT)
		{
			dbus->message_iter_recurse(&args, &variant);
			if (dbus->message_iter_get_arg_type(&variant) == DBUS_TYPE_UINT32)
			{
				dbus->message_iter_get_basic(&variant, &value);
				*out = value;
				result = DARLING_SCREENCAST_OK;
			}
		}
		dbus->message_unref(reply);
	}
	else if (darling_portal_unavailable(error.name))
		result = DARLING_SCREENCAST_NOT_AVAILABLE;

	dbus->error_free(&error);
	return result;
}

int darling_screencast_portal_properties(const struct darling_screencast_portal_api* api,
	uint32_t* source_types, uint32_t* cursor_modes, uint32_t* version)
{
	const struct darling_portal_api* common = portal_api(api);
	const struct darling_dbus* dbus;
	DBusConnection* connection;
	uint32_t value = 0;
	int result;

	if (!darling_portal_api_complete(common))
		return DARLING_SCREENCAST_NOT_AVAILABLE;
	dbus = api->dbus;
	connection = darling_portal_open_session_bus(common);
	if (!connection)
		return DARLING_SCREENCAST_NOT_AVAILABLE;

	if (source_types)
	{
		result = get_uint32_property(common, connection, "AvailableSourceTypes", &value);
		if (result != DARLING_SCREENCAST_OK)
			goto done;
		*source_types = value;
	}
	if (cursor_modes)
	{
		result = get_uint32_property(common, connection, "AvailableCursorModes", &value);
		if (result != DARLING_SCREENCAST_OK)
			goto done;
		*cursor_modes = value;
	}
	if (version)
	{
		result = get_uint32_property(common, connection, "version", &value);
		if (result != DARLING_SCREENCAST_OK)
			goto done;
		*version = value;
	}
	result = DARLING_SCREENCAST_OK;

done:
	dbus->connection_close(connection);
	dbus->connection_unref(connection);
	return result;
}

static void append_create_options(const struct darling_portal_api* api,
	DBusConnection* connection, DBusMessageIter* args, const char* token, void* user_data)
{
	DBusMessageIter options;
	char session_token[96];

	(void) connection; (void) user_data;
	/* CreateSession documents session_handle_token as the name the portal gives the
	 * session; without it the portal frontend has no session name to create. */
	snprintf(session_token, sizeof(session_token), "%s_session", token);
	darling_portal_options_begin(api, args, &options);
	darling_portal_options_add_string(api, &options, "handle_token", token);
	darling_portal_options_add_string(api, &options, "session_handle_token", session_token);
	darling_portal_options_end(api, args, &options);
}

static void append_select_options(const struct darling_portal_api* api,
	DBusConnection* connection, DBusMessageIter* args, const char* token, void* user_data)
{
	const struct darling_screencast_config* config = user_data;
	DBusMessageIter options;
	dbus_bool_t multiple = config->multiple != 0;

	(void) connection;
	darling_portal_options_begin(api, args, &options);
	darling_portal_options_add_string(api, &options, "handle_token", token);
	darling_portal_options_add_uint32(api, &options, "types",
		config->source_types ? config->source_types : DARLING_SCREENCAST_SOURCE_MONITOR);
	if (config->cursor_mode)
		darling_portal_options_add_uint32(api, &options, "cursor_mode", config->cursor_mode);
	darling_portal_options_add_bool(api, &options, "multiple", multiple);
	if (config->persist_mode)
		darling_portal_options_add_uint32(api, &options, "persist_mode", config->persist_mode);
	if (config->restore_token && config->restore_token[0] != '\0')
		darling_portal_options_add_string(api, &options, "restore_token", config->restore_token);
	darling_portal_options_end(api, args, &options);
}

static void append_start_options(const struct darling_portal_api* api,
	DBusConnection* connection, DBusMessageIter* args, const char* token, void* user_data)
{
	DBusMessageIter options;
	char session_token[96];

	(void) connection; (void) user_data;
	/* CreateSession documents session_handle_token as the name the portal gives the
	 * session; without it the portal frontend has no session name to create. */
	snprintf(session_token, sizeof(session_token), "%s_session", token);
	darling_portal_options_begin(api, args, &options);
	darling_portal_options_add_string(api, &options, "handle_token", token);
	darling_portal_options_add_string(api, &options, "session_handle_token", session_token);
	darling_portal_options_end(api, args, &options);
}

/* SelectSources and Start take the session handle before their options dict. */
struct method_context
{
	append_options_fn append;
	void* user_data;
	const char* session_handle;
	int append_parent_window;
};

static void append_session_options(const struct darling_portal_api* api,
	DBusConnection* connection, DBusMessageIter* args, const char* token, void* user_data)
{
	struct method_context* context = user_data;
	const char* empty = "";

	if (context->session_handle)
		api->dbus->message_iter_append_basic(args, DBUS_TYPE_OBJECT_PATH, &context->session_handle);
	if (context->append_parent_window)
		api->dbus->message_iter_append_basic(args, DBUS_TYPE_STRING, &empty);
	context->append(api, connection, args, token, context->user_data);
}

static int run_request(const struct darling_portal_api* api, DBusConnection* connection,
	const char* method, append_options_fn append, void* user_data,
	darling_portal_response_fn response, void* response_data)
{
	const struct darling_dbus* dbus = api->dbus;
	struct method_context context = { append, user_data, NULL, 0 };
	DBusMessage* call;
	DBusMessage* reply;
	DBusMessageIter args;
	DBusError error;
	char token[64];
	const char* handle = NULL;
	char* copy;
	int result;

	darling_portal_make_token(api, connection, "screencast", token, sizeof(token));
	dbus->error_init(&error);
	darling_portal_add_request_match(api, connection, &error);
	if (error.name)
	{
		dbus->error_free(&error);
		return DARLING_SCREENCAST_FAILED;
	}

	call = dbus->message_new_method_call(DARLING_PORTAL_NAME, DARLING_PORTAL_PATH,
		SCREENCAST_INTERFACE, method);
	if (!call)
	{
		dbus->error_free(&error);
		return DARLING_SCREENCAST_FAILED;
	}
	dbus->message_iter_init_append(call, &args);
	append_session_options(api, connection, &args, token, &context);
	reply = dbus->connection_send_with_reply_and_block(connection, call,
		darling_portal_timeout_ms(api), &error);
	dbus->message_unref(call);

	if (!reply)
	{
		result = darling_portal_unavailable(error.name)
			? DARLING_SCREENCAST_NOT_AVAILABLE : DARLING_SCREENCAST_FAILED;
		dbus->error_free(&error);
		return result;
	}
	if (dbus->message_iter_init(reply, &args)
		&& dbus->message_iter_get_arg_type(&args) == DBUS_TYPE_OBJECT_PATH)
		dbus->message_iter_get_basic(&args, &handle);
	copy = handle && strlen(handle) < MAX_HANDLE_LENGTH ? strdup(handle) : NULL;
	dbus->message_unref(reply);
	dbus->error_free(&error);

	if (!copy)
		return DARLING_SCREENCAST_FAILED;
	result = darling_portal_wait_response(api, connection, copy, response, response_data);
	free(copy);
	return result;
}

static int run_session_request(const struct darling_portal_api* api, DBusConnection* connection,
	const char* method, const char* session_handle, append_options_fn append,
	void* user_data, darling_portal_response_fn response, void* response_data)
{
	struct method_context context = { append, user_data, session_handle, 0 };

	return run_request(api, connection, method, append_session_options, &context,
		response, response_data);
}

static void on_session_created(DBusMessageIter* args, void* user_data)
{
	struct handle_context* context = user_data;
	const struct darling_dbus* dbus = context->dbus;
	DBusMessageIter options, entry, variant;
	const char* key = NULL;
	const char* value = NULL;

	if (!dbus->message_iter_next(args))
		return;
	if (dbus->message_iter_get_arg_type(args) != DBUS_TYPE_ARRAY)
		return;
	dbus->message_iter_recurse(args, &options);
	while (dbus->message_iter_get_arg_type(&options) == DBUS_TYPE_DICT_ENTRY)
	{
		dbus->message_iter_recurse(&options, &entry);
		if (dbus->message_iter_get_arg_type(&entry) == DBUS_TYPE_STRING)
			dbus->message_iter_get_basic(&entry, &key);
		if (dbus->message_iter_next(&entry)
			&& dbus->message_iter_get_arg_type(&entry) == DBUS_TYPE_VARIANT)
		{
			dbus->message_iter_recurse(&entry, &variant);
			if (dbus->message_iter_get_arg_type(&variant) == DBUS_TYPE_STRING)
				dbus->message_iter_get_basic(&variant, &value);
		}
		if (key && value && strcmp(key, "session_handle") == 0
			&& value[0] == '/' && strlen(value) < MAX_HANDLE_LENGTH)
		{
			snprintf(context->handle, sizeof(context->handle), "%s", value);
			return;
		}
		dbus->message_iter_next(&options);
	}
}

static int read_int_pair(const struct darling_dbus* dbus, DBusMessageIter* variant,
	int32_t* first, int32_t* second)
{
	DBusMessageIter tuple;

	if (dbus->message_iter_get_arg_type(variant) != DBUS_TYPE_STRUCT)
		return 0;
	dbus->message_iter_recurse(variant, &tuple);
	if (dbus->message_iter_get_arg_type(&tuple) != DBUS_TYPE_INT32)
		return 0;
	dbus->message_iter_get_basic(&tuple, first);
	if (!dbus->message_iter_next(&tuple)
		|| dbus->message_iter_get_arg_type(&tuple) != DBUS_TYPE_INT32)
		return 0;
	dbus->message_iter_get_basic(&tuple, second);
	return 1;
}

static void parse_stream_properties(struct stream_parse_context* context,
	DBusMessageIter* properties, struct darling_screencast_stream* stream)
{
	const struct darling_dbus* dbus = context->dbus;
	DBusMessageIter entries, entry, variant;
	const char* key = NULL;
	dbus_uint32_t source_type = 0;
	dbus_uint64_t serial = 0;
	int32_t first = 0, second = 0;

	stream->source_type = context->source_types;
	if (dbus->message_iter_get_arg_type(properties) != DBUS_TYPE_ARRAY)
		return;
	dbus->message_iter_recurse(properties, &entries);
	while (dbus->message_iter_get_arg_type(&entries) == DBUS_TYPE_DICT_ENTRY)
	{
		dbus->message_iter_recurse(&entries, &entry);
		if (dbus->message_iter_get_arg_type(&entry) == DBUS_TYPE_STRING)
			dbus->message_iter_get_basic(&entry, &key);
		if (dbus->message_iter_next(&entry)
			&& dbus->message_iter_get_arg_type(&entry) == DBUS_TYPE_VARIANT)
		{
			dbus->message_iter_recurse(&entry, &variant);
			if (key && strcmp(key, "size") == 0
				&& read_int_pair(dbus, &variant, &first, &second))
			{
				stream->width = first;
				stream->height = second;
			}
			else if (key && strcmp(key, "position") == 0
				&& read_int_pair(dbus, &variant, &first, &second))
			{
				stream->x = first;
				stream->y = second;
			}
			else if (key && strcmp(key, "source_type") == 0
				&& dbus->message_iter_get_arg_type(&variant) == DBUS_TYPE_UINT32)
			{
				dbus->message_iter_get_basic(&variant, &source_type);
				stream->source_type = source_type;
			}
			else if (key && strcmp(key, "pipewire-serial") == 0
				&& dbus->message_iter_get_arg_type(&variant) == DBUS_TYPE_UINT64)
			{
				dbus->message_iter_get_basic(&variant, &serial);
				stream->serial = serial;
			}
			else if (key && strcmp(key, "id") == 0
				&& dbus->message_iter_get_arg_type(&variant) == DBUS_TYPE_STRING)
			{
				const char* id = NULL;
				dbus->message_iter_get_basic(&variant, &id);
				if (id)
					snprintf(stream->id, sizeof(stream->id), "%s", id);
			}
		}
		dbus->message_iter_next(&entries);
	}

	if (stream->serial)
		snprintf(stream->target_object, sizeof(stream->target_object), "%llu",
			(unsigned long long) stream->serial);
	else
		snprintf(stream->target_object, sizeof(stream->target_object), "%u", stream->node_id);
}

static void on_started(DBusMessageIter* args, void* user_data)
{
	struct stream_parse_context* context = user_data;
	const struct darling_dbus* dbus = context->dbus;
	DBusMessageIter options, entry, variant, streams, stream;

	if (!dbus->message_iter_next(args)
		|| dbus->message_iter_get_arg_type(args) != DBUS_TYPE_ARRAY)
		return;
	dbus->message_iter_recurse(args, &options);
	while (dbus->message_iter_get_arg_type(&options) == DBUS_TYPE_DICT_ENTRY)
	{
		dbus->message_iter_recurse(&options, &entry);
		if (dbus->message_iter_get_arg_type(&entry) != DBUS_TYPE_STRING)
		{
			dbus->message_iter_next(&options);
			continue;
		}
		{
			const char* key = NULL;
			dbus->message_iter_get_basic(&entry, &key);
			if (!dbus->message_iter_next(&entry)
				|| dbus->message_iter_get_arg_type(&entry) != DBUS_TYPE_VARIANT
				|| !key || strcmp(key, "streams") != 0)
			{
				dbus->message_iter_next(&options);
				continue;
			}
			dbus->message_iter_recurse(&entry, &variant);
			if (dbus->message_iter_get_arg_type(&variant) != DBUS_TYPE_ARRAY)
				return;
			dbus->message_iter_recurse(&variant, &streams);
			while (dbus->message_iter_get_arg_type(&streams) == DBUS_TYPE_STRUCT
				&& context->count < MAX_STREAMS)
			{
				dbus->message_iter_recurse(&streams, &stream);
				if (dbus->message_iter_get_arg_type(&stream) != DBUS_TYPE_UINT32)
					break;
				{
					dbus_uint32_t node_id = 0;
					dbus->message_iter_get_basic(&stream, &node_id);
					if (!dbus->message_iter_next(&stream)
						|| dbus->message_iter_get_arg_type(&stream) != DBUS_TYPE_ARRAY)
						break;
					context->streams[context->count].node_id = node_id;
					parse_stream_properties(context, &stream, &context->streams[context->count]);
					context->count++;
				}
				dbus->message_iter_next(&streams);
			}
			return;
		}
	}
}

static int open_remote(const struct darling_portal_api* api, DBusConnection* connection,
	const char* session_handle, int* fd_out)
{
	const struct darling_dbus* dbus = api->dbus;
	DBusMessage* call;
	DBusMessage* reply;
	DBusMessageIter args;
	DBusError error;
	int fd = -1;
	int unavailable;

	call = dbus->message_new_method_call(DARLING_PORTAL_NAME, DARLING_PORTAL_PATH,
		SCREENCAST_INTERFACE, "OpenPipeWireRemote");
	if (!call)
		return DARLING_SCREENCAST_FAILED;
	dbus->message_iter_init_append(call, &args);
	if (!dbus->message_iter_append_basic(&args, DBUS_TYPE_OBJECT_PATH, &session_handle))
	{
		dbus->message_unref(call);
		return DARLING_SCREENCAST_FAILED;
	}
	darling_portal_append_empty_options(api, &args);

	dbus->error_init(&error);
	reply = dbus->connection_send_with_reply_and_block(connection, call,
		darling_portal_timeout_ms(api), &error);
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
	unavailable = darling_portal_unavailable(error.name);
	dbus->error_free(&error);

	if (fd < 0)
		return unavailable ? DARLING_SCREENCAST_NOT_AVAILABLE : DARLING_SCREENCAST_FAILED;
	*fd_out = fd;
	return DARLING_SCREENCAST_OK;
}

void darling_screencast_session_destroy(struct darling_screencast_session* session)
{
	const struct darling_dbus* dbus;
	DBusError error;

	if (!session)
		return;
	dbus = session->api.dbus;
	if (session->handle[0] == '/')
	{
		DBusMessage* call = dbus->message_new_method_call(DARLING_PORTAL_NAME,
			session->handle, SESSION_INTERFACE, "Close");
		if (call)
		{
			dbus->error_init(&error);
			{
				DBusMessage* reply = dbus->connection_send_with_reply_and_block(
					session->connection, call, darling_portal_timeout_ms(&session->api), &error);
				if (reply)
					dbus->message_unref(reply);
			}
			dbus->error_free(&error);
			dbus->message_unref(call);
		}
	}
	if (session->connection)
	{
		dbus->connection_close(session->connection);
		dbus->connection_unref(session->connection);
	}
	if (session->fd >= 0)
		close(session->fd);
	free(session->streams);
	free(session);
}

int darling_screencast_session_create(const struct darling_screencast_portal_api* api,
	const struct darling_screencast_config* config, struct darling_screencast_session** out)
{
	const struct darling_portal_api* common = portal_api(api);
	const struct darling_dbus* dbus;
	const struct darling_screencast_config default_config = { 0, 0, 0, 0, NULL };
	struct darling_screencast_config chosen = default_config;
	struct darling_screencast_session* session;
	struct handle_context created = { 0 };
	struct stream_parse_context streams;
	DBusConnection* connection;
	int result;

	if (!out)
		return DARLING_SCREENCAST_FAILED;
	*out = NULL;
	if (!darling_portal_api_complete(common))
		return DARLING_SCREENCAST_NOT_AVAILABLE;
	if (config)
		chosen = *config;
	if (chosen.source_types == 0)
		chosen.source_types = DARLING_SCREENCAST_SOURCE_MONITOR;
	if (chosen.cursor_mode == 0)
		chosen.cursor_mode = DARLING_SCREENCAST_CURSOR_HIDDEN;

	dbus = api->dbus;
	connection = darling_portal_open_session_bus(common);
	if (!connection)
		return DARLING_SCREENCAST_NOT_AVAILABLE;

	session = calloc(1, sizeof(*session));
	if (!session)
	{
		dbus->connection_close(connection);
		dbus->connection_unref(connection);
		return DARLING_SCREENCAST_FAILED;
	}
	session->api = *common;
	session->dbus = *dbus;
	session->api.dbus = &session->dbus;
	session->connection = connection;
	session->fd = -1;

	created.dbus = dbus;
	result = run_request(common, connection, "CreateSession", append_create_options,
		NULL, on_session_created, &created);
	if (result == DARLING_SCREENCAST_OK && created.handle[0] == '\0')
		result = DARLING_SCREENCAST_FAILED;
	if (result == DARLING_SCREENCAST_OK)
		snprintf(session->handle, sizeof(session->handle), "%s", created.handle);
	if (result != DARLING_SCREENCAST_OK)
		goto fail;

	result = run_session_request(common, connection, "SelectSources", session->handle,
		append_select_options, &chosen, NULL, NULL);
	if (result != DARLING_SCREENCAST_OK)
		goto fail;

	memset(&streams, 0, sizeof(streams));
	streams.dbus = dbus;
	streams.source_types = chosen.source_types;
	if ((chosen.source_types & (chosen.source_types - 1)) != 0)
		streams.source_types = DARLING_SCREENCAST_SOURCE_MONITOR;
	streams.streams = calloc(MAX_STREAMS, sizeof(*streams.streams));
	if (!streams.streams)
	{
		result = DARLING_SCREENCAST_FAILED;
		goto fail;
	}
	{
		struct method_context start = { append_start_options, NULL, session->handle, 1 };
		result = run_request(common, connection, "Start", append_session_options, &start,
			on_started, &streams);
	}
	if (result != DARLING_SCREENCAST_OK || streams.count == 0)
	{
		free(streams.streams);
		result = result == DARLING_SCREENCAST_OK ? DARLING_SCREENCAST_NO_STREAMS : result;
		goto fail;
	}
	session->streams = realloc(streams.streams, streams.count * sizeof(*session->streams));
	if (session->streams)
		session->stream_count = streams.count;
	else
	{
		free(streams.streams);
		result = DARLING_SCREENCAST_FAILED;
		goto fail;
	}

	result = open_remote(common, connection, session->handle, &session->fd);
	if (result != DARLING_SCREENCAST_OK)
		goto fail;

	*out = session;
	return DARLING_SCREENCAST_OK;

fail:
	darling_screencast_session_destroy(session);
	return result;
}

int darling_screencast_session_fd(const struct darling_screencast_session* session)
{
	return session ? session->fd : -1;
}

const struct darling_screencast_stream* darling_screencast_session_streams(
	const struct darling_screencast_session* session, uint32_t* count)
{
	if (count)
		*count = session ? session->stream_count : 0;
	return session ? session->streams : NULL;
}

const char* darling_screencast_strerror(int result)
{
	switch (result)
	{
		case DARLING_SCREENCAST_OK: return "success";
		case DARLING_SCREENCAST_NOT_AVAILABLE: return "screen cast portal unavailable";
		case DARLING_SCREENCAST_DENIED: return "screen cast permission denied";
		case DARLING_SCREENCAST_FAILED: return "screen cast portal call failed";
		case DARLING_SCREENCAST_CANCELLED: return "screen cast request cancelled";
		case DARLING_SCREENCAST_NO_STREAMS: return "screen cast session has no streams";
		default: return "unknown screen cast portal error";
	}
}
