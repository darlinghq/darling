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

#ifndef DARLING_PORTAL_COMMON_H_
#define DARLING_PORTAL_COMMON_H_

#include <darling_dbus.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The shared prefix of every portal client's injected API. Portal clients may
 * embed this at the start of their own API structure. */
struct darling_portal_api
{
	const struct darling_dbus* dbus;
	char* (*getenv)(const char* name);
	int timeout_ms;
};

#define DARLING_PORTAL_NAME "org.freedesktop.portal.Desktop"
#define DARLING_PORTAL_PATH "/org/freedesktop/portal/desktop"
#define DARLING_REQUEST_INTERFACE "org.freedesktop.portal.Request"
#define DARLING_SESSION_INTERFACE "org.freedesktop.portal.Session"
#define DARLING_PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"

int darling_portal_api_complete(const struct darling_portal_api* api);
DBusConnection* darling_portal_open_session_bus(const struct darling_portal_api* api);
int darling_portal_unavailable(const char* name);
int darling_portal_timeout_ms(const struct darling_portal_api* api);

void darling_portal_make_token(const struct darling_portal_api* api,
	DBusConnection* connection, const char* prefix, char* token, size_t size);
void darling_portal_add_request_match(const struct darling_portal_api* api,
	DBusConnection* connection, DBusError* error);
void darling_portal_options_begin(const struct darling_portal_api* api,
	DBusMessageIter* args, DBusMessageIter* options);
void darling_portal_options_end(const struct darling_portal_api* api,
	DBusMessageIter* args, DBusMessageIter* options);
void darling_portal_options_add_string(const struct darling_portal_api* api,
	DBusMessageIter* options, const char* key, const char* value);
void darling_portal_options_add_uint32(const struct darling_portal_api* api,
	DBusMessageIter* options, const char* key, dbus_uint32_t value);
void darling_portal_options_add_bool(const struct darling_portal_api* api,
	DBusMessageIter* options, const char* key, dbus_bool_t value);
void darling_portal_append_empty_options(const struct darling_portal_api* api,
	DBusMessageIter* args);
void darling_portal_append_handle_token(const struct darling_portal_api* api,
	DBusMessageIter* args, const char* token);
void darling_portal_append_string_option(const struct darling_portal_api* api,
	DBusMessageIter* args, const char* key, const char* value);
void darling_portal_append_uint32_option(const struct darling_portal_api* api,
	DBusMessageIter* args, const char* key, dbus_uint32_t value);
void darling_portal_append_bool_option(const struct darling_portal_api* api,
	DBusMessageIter* args, const char* key, dbus_bool_t value);

/* Waits for the Request.Response matching handle. The callback receives the
 * iterator positioned at the response code and may read the response options. */
typedef void (*darling_portal_response_fn)(DBusMessageIter* args, void* user_data);
int darling_portal_wait_response(const struct darling_portal_api* api,
	DBusConnection* connection, const char* handle,
	darling_portal_response_fn callback, void* user_data);

#ifdef __cplusplus
}
#endif

#endif
