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

#include "locationd_dbus.h"
#include <dbus/dbus.h>
#include <darling_dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#define MAX_ADDRESS_LENGTH 1024

#define PORTAL_NAME "org.freedesktop.portal.Desktop"
#define PORTAL_PATH "/org/freedesktop/portal/desktop"
#define PORTAL_LOCATION_INTERFACE "org.freedesktop.portal.Location"
#define PORTAL_SESSION_INTERFACE "org.freedesktop.portal.Session"
#define PORTAL_REQUEST_INTERFACE "org.freedesktop.portal.Request"

#define GEOCLUE_NAME "org.freedesktop.GeoClue2"
#define GEOCLUE_MANAGER_PATH "/org/freedesktop/GeoClue2/Manager"
#define GEOCLUE_MANAGER_INTERFACE "org.freedesktop.GeoClue2.Manager"
#define GEOCLUE_CLIENT_INTERFACE "org.freedesktop.GeoClue2.Client"
#define GEOCLUE_LOCATION_INTERFACE "org.freedesktop.GeoClue2.Location"

static const struct darling_dbus *s_api = NULL;

static DBusConnection* s_conn = NULL;
static enum locationd_backend_type s_backend = LOCATIOND_BACKEND_NONE;
static enum locationd_auth_state s_auth_state = LOCATIOND_AUTH_NOT_DETERMINED;

static char* s_portal_session_handle = NULL;
static char* s_portal_request_handle = NULL;

static char* s_geoclue_client_path = NULL;

static locationd_on_fix_fn s_on_fix_cb = NULL;
static locationd_on_auth_fn s_on_auth_cb = NULL;
static locationd_on_error_fn s_on_error_cb = NULL;
static void* s_cb_context = NULL;

static unsigned int s_token_counter = 1;
static int s_is_active = 0;

static int load_dbus_symbols(void)
{
    if (s_api == NULL)
        s_api = darling_dbus_get(DARLING_DBUS_HOST_FALLBACK);
    return s_api != NULL ? 0 : -1;
}

static char* get_session_bus_address(const char* explicit_addr)
{
    if (explicit_addr && explicit_addr[0] != '\0')
        return strdup(explicit_addr);

    const char* env = getenv("__LOCATIOND_DBUS_ADDRESS");
    if (!env || env[0] == '\0')
        env = getenv("DBUS_SESSION_BUS_ADDRESS");
    if (env && env[0] != '\0')
        return strdup(env);

    const char* runtime_dir = getenv("XDG_RUNTIME_DIR");
    if (runtime_dir && runtime_dir[0] == '/') {
        char path[MAX_ADDRESS_LENGTH];
        if (snprintf(path, sizeof(path), "%s/bus", runtime_dir) < (int)sizeof(path)) {
            if (access(path, R_OK | W_OK) == 0) {
                char* escaped = s_api->address_escape_value(path);
                if (escaped) {
                    size_t len = strlen("unix:path=") + strlen(escaped) + 1;
                    char* addr = (char*)malloc(len);
                    if (addr)
                        snprintf(addr, len, "unix:path=%s", escaped);
                    s_api->free(escaped);
                    return addr;
                }
            }
        }
    }
    return NULL;
}

static char* get_system_bus_address(const char* explicit_addr)
{
    if (explicit_addr && explicit_addr[0] != '\0')
        return strdup(explicit_addr);

    const char* env = getenv("__LOCATIOND_SYSTEM_DBUS_ADDRESS");
    if (!env || env[0] == '\0')
        env = getenv("DBUS_SYSTEM_BUS_ADDRESS");
    if (env && env[0] != '\0')
        return strdup(env);

    const char* default_path = "/var/run/dbus/system_bus_socket";
    if (access(default_path, R_OK | W_OK) == 0)
        return strdup("unix:path=/var/run/dbus/system_bus_socket");

    return NULL;
}

static uint32_t map_accuracy_to_portal(double desired_accuracy)
{
    if (desired_accuracy < 0.0 || desired_accuracy <= 10.0)
        return 5; // EXACT
    if (desired_accuracy <= 100.0)
        return 4; // STREET
    if (desired_accuracy <= 1000.0)
        return 3; // NEIGHBORHOOD
    if (desired_accuracy <= 3000.0)
        return 2; // CITY
    return 1;     // COUNTRY
}

static uint32_t map_accuracy_to_geoclue(double desired_accuracy)
{
    if (desired_accuracy < 0.0 || desired_accuracy <= 10.0)
        return 8; // EXACT
    if (desired_accuracy <= 100.0)
        return 6; // STREET
    if (desired_accuracy <= 1000.0)
        return 5; // NEIGHBORHOOD
    if (desired_accuracy <= 3000.0)
        return 4; // CITY
    return 1;     // COUNTRY
}

static void append_dict_string(DBusMessageIter* dict, const char* key, const char* val)
{
    DBusMessageIter entry, var;
    s_api->message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    s_api->message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
    s_api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &var);
    s_api->message_iter_append_basic(&var, DBUS_TYPE_STRING, &val);
    s_api->message_iter_close_container(&entry, &var);
    s_api->message_iter_close_container(dict, &entry);
}

static void append_dict_uint32(DBusMessageIter* dict, const char* key, uint32_t val)
{
    DBusMessageIter entry, var;
    s_api->message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    s_api->message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
    s_api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "u", &var);
    s_api->message_iter_append_basic(&var, DBUS_TYPE_UINT32, &val);
    s_api->message_iter_close_container(&entry, &var);
    s_api->message_iter_close_container(dict, &entry);
}

static int check_has_owner(DBusConnection* conn, const char* service_name)
{
    DBusError err;
    s_api->error_init(&err);

    DBusMessage* msg = s_api->message_new_method_call(
        "org.freedesktop.DBus", "/org/freedesktop/DBus",
        "org.freedesktop.DBus", "GetNameOwner");
    if (!msg)
        return 0;

    DBusMessageIter iter;
    s_api->message_iter_init_append(msg, &iter);
    s_api->message_iter_append_basic(&iter, DBUS_TYPE_STRING, &service_name);

    DBusMessage* reply = s_api->connection_send_with_reply_and_block(conn, msg, 1000, &err);
    s_api->message_unref(msg);

    if (reply) {
        s_api->message_unref(reply);
        s_api->error_free(&err);
        return 1;
    }

    s_api->error_free(&err);
    return 0;
}

int locationd_dbus_init(const char* session_bus_address, const char* system_bus_address)
{
    locationd_dbus_close();

    if (load_dbus_symbols() != 0)
        return -1;

    // 1. Try Session bus and xdg-desktop-portal
    char* s_addr = get_session_bus_address(session_bus_address);
    if (s_addr) {
        DBusError err;
        s_api->error_init(&err);
        DBusConnection* sc = s_api->connection_open_private(s_addr, &err);
        free(s_addr);

        if (sc && s_api->bus_register(sc, &err)) {
            // Check if portal service is available
            if (check_has_owner(sc, PORTAL_NAME)) {
                s_conn = sc;
                s_backend = LOCATIOND_BACKEND_PORTAL;
                s_auth_state = LOCATIOND_AUTH_NOT_DETERMINED;
                s_api->error_free(&err);
                return LOCATIOND_BACKEND_PORTAL;
            }
            // Portal not available on this session bus; close and try system bus
            s_api->connection_close(sc);
            s_api->connection_unref(sc);
        } else if (sc) {
            s_api->connection_close(sc);
            s_api->connection_unref(sc);
        }
        s_api->error_free(&err);
    }

    // 2. Try System bus and GeoClue2
    char* sys_addr = get_system_bus_address(system_bus_address);
    if (sys_addr) {
        DBusError err;
        s_api->error_init(&err);
        DBusConnection* sys_c = s_api->connection_open_private(sys_addr, &err);
        free(sys_addr);

        if (sys_c && s_api->bus_register(sys_c, &err)) {
            if (check_has_owner(sys_c, GEOCLUE_NAME)) {
                s_conn = sys_c;
                s_backend = LOCATIOND_BACKEND_GEOCLUE;
                s_auth_state = LOCATIOND_AUTH_NOT_DETERMINED;
                s_api->error_free(&err);
                return LOCATIOND_BACKEND_GEOCLUE;
            }
            s_api->connection_close(sys_c);
            s_api->connection_unref(sys_c);
        } else if (sys_c) {
            s_api->connection_close(sys_c);
            s_api->connection_unref(sys_c);
        }
        s_api->error_free(&err);
    }

    s_backend = LOCATIOND_BACKEND_NONE;
    s_auth_state = LOCATIOND_AUTH_RESTRICTED;
    return LOCATIOND_BACKEND_NONE;
}

int locationd_dbus_is_available(void)
{
    return s_backend != LOCATIOND_BACKEND_NONE && s_conn != NULL;
}

enum locationd_backend_type locationd_dbus_get_backend(void)
{
    return s_backend;
}

int locationd_dbus_get_fd(void)
{
    if (!s_conn)
        return -1;
    int fd = -1;
    if (s_api->connection_get_unix_fd(s_conn, &fd) && fd >= 0)
        return fd;
    return -1;
}

static void parse_location_dict(DBusMessageIter* dict, struct locationd_fix* fix)
{
    memset(fix, 0, sizeof(*fix));
    fix->vertical_accuracy = -1.0;
    fix->speed = -1.0;
    fix->course = -1.0;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    fix->timestamp = (double)tv.tv_sec + ((double)tv.tv_usec / 1000000.0);

    DBusMessageIter entry;
    s_api->message_iter_recurse(dict, &entry);
    while (s_api->message_iter_get_arg_type(&entry) == DBUS_TYPE_DICT_ENTRY) {
        DBusMessageIter kv, var;
        s_api->message_iter_recurse(&entry, &kv);
        const char* key = NULL;
        s_api->message_iter_get_basic(&kv, &key);
        s_api->message_iter_next(&kv);
        s_api->message_iter_recurse(&kv, &var);

        int var_type = s_api->message_iter_get_arg_type(&var);
        if (key) {
            if (strcmp(key, "Latitude") == 0 && var_type == DBUS_TYPE_DOUBLE) {
                s_api->message_iter_get_basic(&var, &fix->latitude);
            } else if (strcmp(key, "Longitude") == 0 && var_type == DBUS_TYPE_DOUBLE) {
                s_api->message_iter_get_basic(&var, &fix->longitude);
            } else if (strcmp(key, "Altitude") == 0 && var_type == DBUS_TYPE_DOUBLE) {
                double alt = 0.0;
                s_api->message_iter_get_basic(&var, &alt);
                if (alt > -1e300) {
                    fix->altitude = alt;
                    fix->has_altitude = 1;
                    fix->vertical_accuracy = 10.0;
                }
            } else if (strcmp(key, "Accuracy") == 0 && var_type == DBUS_TYPE_DOUBLE) {
                s_api->message_iter_get_basic(&var, &fix->horizontal_accuracy);
            } else if (strcmp(key, "Speed") == 0 && var_type == DBUS_TYPE_DOUBLE) {
                double spd = -1.0;
                s_api->message_iter_get_basic(&var, &spd);
                if (spd >= 0.0) {
                    fix->speed = spd;
                    fix->has_speed = 1;
                }
            } else if (strcmp(key, "Heading") == 0 && var_type == DBUS_TYPE_DOUBLE) {
                double hd = -1.0;
                s_api->message_iter_get_basic(&var, &hd);
                if (hd >= 0.0) {
                    fix->course = hd;
                    fix->has_course = 1;
                }
            } else if (strcmp(key, "Timestamp") == 0) {
                if (var_type == DBUS_TYPE_STRUCT) {
                    DBusMessageIter st;
                    s_api->message_iter_recurse(&var, &st);
                    uint64_t sec = 0, usec = 0;
                    if (s_api->message_iter_get_arg_type(&st) == DBUS_TYPE_UINT64) {
                        s_api->message_iter_get_basic(&st, &sec);
                        s_api->message_iter_next(&st);
                        if (s_api->message_iter_get_arg_type(&st) == DBUS_TYPE_UINT64)
                            s_api->message_iter_get_basic(&st, &usec);
                        if (sec > 0)
                            fix->timestamp = (double)sec + ((double)usec / 1000000.0);
                    }
                }
            }
        }
        if (!s_api->message_iter_next(&entry))
            break;
    }
}

static int start_portal(double desired_accuracy, double distance_threshold)
{
    DBusError err;
    s_api->error_init(&err);

    // CreateSession
    DBusMessage* msg = s_api->message_new_method_call(
        PORTAL_NAME, PORTAL_PATH, PORTAL_LOCATION_INTERFACE, "CreateSession");
    if (!msg)
        return -1;

    char token[64];
    snprintf(token, sizeof(token), "darling_sess_%d_%u", (int)getpid(), s_token_counter++);

    DBusMessageIter iter, dict;
    s_api->message_iter_init_append(msg, &iter);
    s_api->message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict);
    append_dict_string(&dict, "session_handle_token", token);
    append_dict_uint32(&dict, "distance-threshold", (distance_threshold > 0) ? (uint32_t)distance_threshold : 0);
    append_dict_uint32(&dict, "time-threshold", 0);
    append_dict_uint32(&dict, "accuracy", map_accuracy_to_portal(desired_accuracy));
    s_api->message_iter_close_container(&iter, &dict);

    DBusMessage* reply = s_api->connection_send_with_reply_and_block(s_conn, msg, 5000, &err);
    s_api->message_unref(msg);

    if (!reply) {
        s_api->error_free(&err);
        return -2;
    }

    s_api->message_iter_init(reply, &iter);
    const char* handle = NULL;
    if (s_api->message_iter_get_arg_type(&iter) == DBUS_TYPE_OBJECT_PATH)
        s_api->message_iter_get_basic(&iter, &handle);

    if (!handle) {
        s_api->message_unref(reply);
        s_api->error_free(&err);
        return -3;
    }

    free(s_portal_session_handle);
    s_portal_session_handle = strdup(handle);
    s_api->message_unref(reply);

    // Add match for LocationUpdated and Response signals
    s_api->bus_add_match(s_conn,
        "type='signal',sender='org.freedesktop.portal.Desktop',interface='org.freedesktop.portal.Location',member='LocationUpdated'", &err);
    s_api->bus_add_match(s_conn,
        "type='signal',sender='org.freedesktop.portal.Desktop',interface='org.freedesktop.portal.Request',member='Response'", &err);
    s_api->bus_add_match(s_conn,
        "type='signal',sender='org.freedesktop.portal.Desktop',interface='org.freedesktop.portal.Session',member='Closed'", &err);

    // Call Start(session_handle, "", options)
    msg = s_api->message_new_method_call(
        PORTAL_NAME, PORTAL_PATH, PORTAL_LOCATION_INTERFACE, "Start");
    if (!msg)
        return -4;

    char start_token[64];
    snprintf(start_token, sizeof(start_token), "darling_start_%d_%u", (int)getpid(), s_token_counter++);
    const char* parent_window = "";

    s_api->message_iter_init_append(msg, &iter);
    s_api->message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &s_portal_session_handle);
    s_api->message_iter_append_basic(&iter, DBUS_TYPE_STRING, &parent_window);
    s_api->message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict);
    append_dict_string(&dict, "handle_token", start_token);
    s_api->message_iter_close_container(&iter, &dict);

    reply = s_api->connection_send_with_reply_and_block(s_conn, msg, 5000, &err);
    s_api->message_unref(msg);

    if (!reply) {
        s_api->error_free(&err);
        return -5;
    }

    s_api->message_iter_init(reply, &iter);
    const char* req_handle = NULL;
    if (s_api->message_iter_get_arg_type(&iter) == DBUS_TYPE_OBJECT_PATH)
        s_api->message_iter_get_basic(&iter, &req_handle);

    free(s_portal_request_handle);
    s_portal_request_handle = req_handle ? strdup(req_handle) : NULL;
    s_api->message_unref(reply);
    s_api->error_free(&err);

    return 0;
}

static int start_geoclue(double desired_accuracy, double distance_threshold)
{
    DBusError err;
    s_api->error_init(&err);

    // GetClient
    DBusMessage* msg = s_api->message_new_method_call(
        GEOCLUE_NAME, GEOCLUE_MANAGER_PATH, GEOCLUE_MANAGER_INTERFACE, "GetClient");
    if (!msg)
        return -1;

    DBusMessage* reply = s_api->connection_send_with_reply_and_block(s_conn, msg, 5000, &err);
    s_api->message_unref(msg);

    if (!reply) {
        s_api->error_free(&err);
        return -2;
    }

    DBusMessageIter iter;
    s_api->message_iter_init(reply, &iter);
    const char* client_path = NULL;
    if (s_api->message_iter_get_arg_type(&iter) == DBUS_TYPE_OBJECT_PATH)
        s_api->message_iter_get_basic(&iter, &client_path);

    if (!client_path) {
        s_api->message_unref(reply);
        s_api->error_free(&err);
        return -3;
    }

    free(s_geoclue_client_path);
    s_geoclue_client_path = strdup(client_path);
    s_api->message_unref(reply);

    // Set DesktopId property
    msg = s_api->message_new_method_call(
        GEOCLUE_NAME, s_geoclue_client_path, "org.freedesktop.DBus.Properties", "Set");
    if (msg) {
        const char* iface = GEOCLUE_CLIENT_INTERFACE;
        const char* prop = "DesktopId";
        const char* val = "com.apple.locationd";
        DBusMessageIter var;
        s_api->message_iter_init_append(msg, &iter);
        s_api->message_iter_append_basic(&iter, DBUS_TYPE_STRING, &iface);
        s_api->message_iter_append_basic(&iter, DBUS_TYPE_STRING, &prop);
        s_api->message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "s", &var);
        s_api->message_iter_append_basic(&var, DBUS_TYPE_STRING, &val);
        s_api->message_iter_close_container(&iter, &var);
        reply = s_api->connection_send_with_reply_and_block(s_conn, msg, 2000, &err);
        s_api->message_unref(msg);
        if (reply) s_api->message_unref(reply);
        s_api->error_free(&err);
    }

    // Set DistanceThreshold property
    msg = s_api->message_new_method_call(
        GEOCLUE_NAME, s_geoclue_client_path, "org.freedesktop.DBus.Properties", "Set");
    if (msg) {
        const char* iface = GEOCLUE_CLIENT_INTERFACE;
        const char* prop = "DistanceThreshold";
        uint32_t dist = (distance_threshold > 0) ? (uint32_t)distance_threshold : 0;
        DBusMessageIter var;
        s_api->message_iter_init_append(msg, &iter);
        s_api->message_iter_append_basic(&iter, DBUS_TYPE_STRING, &iface);
        s_api->message_iter_append_basic(&iter, DBUS_TYPE_STRING, &prop);
        s_api->message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "u", &var);
        s_api->message_iter_append_basic(&var, DBUS_TYPE_UINT32, &dist);
        s_api->message_iter_close_container(&iter, &var);
        reply = s_api->connection_send_with_reply_and_block(s_conn, msg, 2000, &err);
        s_api->message_unref(msg);
        if (reply) s_api->message_unref(reply);
        s_api->error_free(&err);
    }

    // Set RequestedAccuracyLevel property
    msg = s_api->message_new_method_call(
        GEOCLUE_NAME, s_geoclue_client_path, "org.freedesktop.DBus.Properties", "Set");
    if (msg) {
        const char* iface = GEOCLUE_CLIENT_INTERFACE;
        const char* prop = "RequestedAccuracyLevel";
        uint32_t acc = map_accuracy_to_geoclue(desired_accuracy);
        DBusMessageIter var;
        s_api->message_iter_init_append(msg, &iter);
        s_api->message_iter_append_basic(&iter, DBUS_TYPE_STRING, &iface);
        s_api->message_iter_append_basic(&iter, DBUS_TYPE_STRING, &prop);
        s_api->message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "u", &var);
        s_api->message_iter_append_basic(&var, DBUS_TYPE_UINT32, &acc);
        s_api->message_iter_close_container(&iter, &var);
        reply = s_api->connection_send_with_reply_and_block(s_conn, msg, 2000, &err);
        s_api->message_unref(msg);
        if (reply) s_api->message_unref(reply);
        s_api->error_free(&err);
    }

    // Add match for LocationUpdated signal from GeoClue client
    char match[512];
    snprintf(match, sizeof(match),
        "type='signal',sender='%s',interface='%s',member='LocationUpdated',path='%s'",
        GEOCLUE_NAME, GEOCLUE_CLIENT_INTERFACE, s_geoclue_client_path);
    s_api->bus_add_match(s_conn, match, &err);
    s_api->error_free(&err);

    // Call Start()
    msg = s_api->message_new_method_call(
        GEOCLUE_NAME, s_geoclue_client_path, GEOCLUE_CLIENT_INTERFACE, "Start");
    if (!msg)
        return -4;

    reply = s_api->connection_send_with_reply_and_block(s_conn, msg, 5000, &err);
    s_api->message_unref(msg);

    if (!reply) {
        // GeoClue rejected authorization or failed
        s_auth_state = LOCATIOND_AUTH_DENIED;
        if (s_on_auth_cb)
            s_on_auth_cb(LOCATIOND_AUTH_DENIED, s_cb_context);
        if (s_on_error_cb)
            s_on_error_cb(1, s_cb_context); // kCLErrorDenied
        s_api->error_free(&err);
        return -5;
    }

    s_api->message_unref(reply);
    s_api->error_free(&err);
    return 0;
}

int locationd_dbus_start(double desired_accuracy, double distance_threshold,
                         locationd_on_fix_fn on_fix, locationd_on_auth_fn on_auth,
                         locationd_on_error_fn on_error, void* context)
{
    s_on_fix_cb = on_fix;
    s_on_auth_cb = on_auth;
    s_on_error_cb = on_error;
    s_cb_context = context;

    if (!s_conn || s_backend == LOCATIOND_BACKEND_NONE) {
        if (s_on_auth_cb)
            s_on_auth_cb(LOCATIOND_AUTH_RESTRICTED, s_cb_context);
        if (s_on_error_cb)
            s_on_error_cb(1, s_cb_context);
        return -1;
    }

    if (s_is_active)
        locationd_dbus_stop();

    int rc = -1;
    if (s_backend == LOCATIOND_BACKEND_PORTAL)
        rc = start_portal(desired_accuracy, distance_threshold);
    else if (s_backend == LOCATIOND_BACKEND_GEOCLUE)
        rc = start_geoclue(desired_accuracy, distance_threshold);

    if (rc == 0) {
        s_is_active = 1;
        s_auth_state = LOCATIOND_AUTH_AUTHORIZED;
        if (s_on_auth_cb)
            s_on_auth_cb(LOCATIOND_AUTH_AUTHORIZED, s_cb_context);
    }

    return rc;
}

void locationd_dbus_stop(void)
{
    if (!s_conn)
        return;

    DBusError err;
    s_api->error_init(&err);

    if (s_backend == LOCATIOND_BACKEND_PORTAL && s_portal_session_handle) {
        DBusMessage* msg = s_api->message_new_method_call(
            PORTAL_NAME, s_portal_session_handle, PORTAL_SESSION_INTERFACE, "Close");
        if (msg) {
            DBusMessage* reply = s_api->connection_send_with_reply_and_block(s_conn, msg, 1000, &err);
            s_api->message_unref(msg);
            if (reply) s_api->message_unref(reply);
        }
        free(s_portal_session_handle);
        s_portal_session_handle = NULL;
        free(s_portal_request_handle);
        s_portal_request_handle = NULL;
    } else if (s_backend == LOCATIOND_BACKEND_GEOCLUE && s_geoclue_client_path) {
        DBusMessage* msg = s_api->message_new_method_call(
            GEOCLUE_NAME, s_geoclue_client_path, GEOCLUE_CLIENT_INTERFACE, "Stop");
        if (msg) {
            DBusMessage* reply = s_api->connection_send_with_reply_and_block(s_conn, msg, 1000, &err);
            s_api->message_unref(msg);
            if (reply) s_api->message_unref(reply);
        }
        msg = s_api->message_new_method_call(
            GEOCLUE_NAME, GEOCLUE_MANAGER_PATH, GEOCLUE_MANAGER_INTERFACE, "DeleteClient");
        if (msg) {
            DBusMessageIter iter;
            s_api->message_iter_init_append(msg, &iter);
            s_api->message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &s_geoclue_client_path);
            DBusMessage* reply = s_api->connection_send_with_reply_and_block(s_conn, msg, 1000, &err);
            s_api->message_unref(msg);
            if (reply) s_api->message_unref(reply);
        }
        free(s_geoclue_client_path);
        s_geoclue_client_path = NULL;
    }

    s_is_active = 0;
    s_api->error_free(&err);
}

int locationd_dbus_update_parameters(double desired_accuracy, double distance_threshold)
{
    if (!s_is_active || !s_conn)
        return -1;

    if (s_backend == LOCATIOND_BACKEND_GEOCLUE && s_geoclue_client_path) {
        DBusError err;
        s_api->error_init(&err);

        // Update DistanceThreshold
        DBusMessage* msg = s_api->message_new_method_call(
            GEOCLUE_NAME, s_geoclue_client_path, "org.freedesktop.DBus.Properties", "Set");
        if (msg) {
            const char* iface = GEOCLUE_CLIENT_INTERFACE;
            const char* prop = "DistanceThreshold";
            uint32_t dist = (distance_threshold > 0) ? (uint32_t)distance_threshold : 0;
            DBusMessageIter iter, var;
            s_api->message_iter_init_append(msg, &iter);
            s_api->message_iter_append_basic(&iter, DBUS_TYPE_STRING, &iface);
            s_api->message_iter_append_basic(&iter, DBUS_TYPE_STRING, &prop);
            s_api->message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "u", &var);
            s_api->message_iter_append_basic(&var, DBUS_TYPE_UINT32, &dist);
            s_api->message_iter_close_container(&iter, &var);
            DBusMessage* reply = s_api->connection_send_with_reply_and_block(s_conn, msg, 1000, &err);
            s_api->message_unref(msg);
            if (reply) s_api->message_unref(reply);
        }

        // Update RequestedAccuracyLevel
        msg = s_api->message_new_method_call(
            GEOCLUE_NAME, s_geoclue_client_path, "org.freedesktop.DBus.Properties", "Set");
        if (msg) {
            const char* iface = GEOCLUE_CLIENT_INTERFACE;
            const char* prop = "RequestedAccuracyLevel";
            uint32_t acc = map_accuracy_to_geoclue(desired_accuracy);
            DBusMessageIter iter, var;
            s_api->message_iter_init_append(msg, &iter);
            s_api->message_iter_append_basic(&iter, DBUS_TYPE_STRING, &iface);
            s_api->message_iter_append_basic(&iter, DBUS_TYPE_STRING, &prop);
            s_api->message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "u", &var);
            s_api->message_iter_append_basic(&var, DBUS_TYPE_UINT32, &acc);
            s_api->message_iter_close_container(&iter, &var);
            DBusMessage* reply = s_api->connection_send_with_reply_and_block(s_conn, msg, 1000, &err);
            s_api->message_unref(msg);
            if (reply) s_api->message_unref(reply);
        }
        s_api->error_free(&err);
        return 0;
    }

    return 0;
}

static void handle_portal_signal(DBusMessage* msg)
{
    if (s_api->message_is_signal(msg, PORTAL_REQUEST_INTERFACE, "Response")) {
        const char* path = s_api->message_get_path(msg);
        if (s_portal_request_handle && path && strcmp(path, s_portal_request_handle) == 0) {
            DBusMessageIter iter;
            s_api->message_iter_init(msg, &iter);
            uint32_t response = 0;
            if (s_api->message_iter_get_arg_type(&iter) == DBUS_TYPE_UINT32)
                s_api->message_iter_get_basic(&iter, &response);

            if (response != 0) {
                s_auth_state = LOCATIOND_AUTH_DENIED;
                if (s_on_auth_cb)
                    s_on_auth_cb(LOCATIOND_AUTH_DENIED, s_cb_context);
                if (s_on_error_cb)
                    s_on_error_cb(1, s_cb_context); // kCLErrorDenied
            }
        }
    } else if (s_api->message_is_signal(msg, PORTAL_LOCATION_INTERFACE, "LocationUpdated")) {
        DBusMessageIter iter;
        s_api->message_iter_init(msg, &iter);
        const char* session_path = NULL;
        if (s_api->message_iter_get_arg_type(&iter) == DBUS_TYPE_OBJECT_PATH) {
            s_api->message_iter_get_basic(&iter, &session_path);
            s_api->message_iter_next(&iter);
        }

        if (s_portal_session_handle && session_path && strcmp(session_path, s_portal_session_handle) == 0) {
            if (s_api->message_iter_get_arg_type(&iter) == DBUS_TYPE_ARRAY) {
                struct locationd_fix fix;
                parse_location_dict(&iter, &fix);

                if (s_auth_state != LOCATIOND_AUTH_AUTHORIZED) {
                    s_auth_state = LOCATIOND_AUTH_AUTHORIZED;
                    if (s_on_auth_cb)
                        s_on_auth_cb(LOCATIOND_AUTH_AUTHORIZED, s_cb_context);
                }
                if (s_on_fix_cb)
                    s_on_fix_cb(&fix, s_cb_context);
            }
        }
    } else if (s_api->message_is_signal(msg, PORTAL_SESSION_INTERFACE, "Closed")) {
        const char* path = s_api->message_get_path(msg);
        if (s_portal_session_handle && path && strcmp(path, s_portal_session_handle) == 0) {
            s_is_active = 0;
        }
    }
}

static void handle_geoclue_signal(DBusMessage* msg)
{
    if (s_api->message_is_signal(msg, GEOCLUE_CLIENT_INTERFACE, "LocationUpdated")) {
        const char* path = s_api->message_get_path(msg);
        if (s_geoclue_client_path && path && strcmp(path, s_geoclue_client_path) == 0) {
            DBusMessageIter iter;
            s_api->message_iter_init(msg, &iter);
            const char* old_loc = NULL;
            const char* new_loc = NULL;
            if (s_api->message_iter_get_arg_type(&iter) == DBUS_TYPE_OBJECT_PATH) {
                s_api->message_iter_get_basic(&iter, &old_loc);
                s_api->message_iter_next(&iter);
                if (s_api->message_iter_get_arg_type(&iter) == DBUS_TYPE_OBJECT_PATH)
                    s_api->message_iter_get_basic(&iter, &new_loc);
            }

            if (new_loc && strcmp(new_loc, "/") != 0) {
                // Query properties on new_loc
                DBusError err;
                s_api->error_init(&err);
                DBusMessage* q_msg = s_api->message_new_method_call(
                    GEOCLUE_NAME, new_loc, "org.freedesktop.DBus.Properties", "GetAll");
                if (q_msg) {
                    const char* iface = GEOCLUE_LOCATION_INTERFACE;
                    DBusMessageIter q_iter;
                    s_api->message_iter_init_append(q_msg, &q_iter);
                    s_api->message_iter_append_basic(&q_iter, DBUS_TYPE_STRING, &iface);
                    DBusMessage* q_reply = s_api->connection_send_with_reply_and_block(s_conn, q_msg, 2000, &err);
                    s_api->message_unref(q_msg);
                    if (q_reply) {
                        DBusMessageIter r_iter;
                        s_api->message_iter_init(q_reply, &r_iter);
                        if (s_api->message_iter_get_arg_type(&r_iter) == DBUS_TYPE_ARRAY) {
                            struct locationd_fix fix;
                            parse_location_dict(&r_iter, &fix);

                            if (s_auth_state != LOCATIOND_AUTH_AUTHORIZED) {
                                s_auth_state = LOCATIOND_AUTH_AUTHORIZED;
                                if (s_on_auth_cb)
                                    s_on_auth_cb(LOCATIOND_AUTH_AUTHORIZED, s_cb_context);
                            }
                            if (s_on_fix_cb)
                                s_on_fix_cb(&fix, s_cb_context);
                        }
                        s_api->message_unref(q_reply);
                    }
                }
                s_api->error_free(&err);
            }
        }
    }
}

int locationd_dbus_process_messages(int timeout_ms)
{
    if (!s_conn)
        return -1;

    if (!s_api->connection_get_is_connected(s_conn))
        return -1;

    s_api->connection_read_write(s_conn, timeout_ms);

    int count = 0;
    DBusMessage* msg = NULL;
    while ((msg = s_api->connection_pop_message(s_conn)) != NULL) {
        count++;
        if (s_backend == LOCATIOND_BACKEND_PORTAL)
            handle_portal_signal(msg);
        else if (s_backend == LOCATIOND_BACKEND_GEOCLUE)
            handle_geoclue_signal(msg);
        s_api->message_unref(msg);
    }

    if (!s_api->connection_get_is_connected(s_conn))
        return -1;

    return count;
}

void locationd_dbus_close(void)
{
    locationd_dbus_stop();

    if (s_conn) {
        s_api->connection_close(s_conn);
        s_api->connection_unref(s_conn);
        s_conn = NULL;
    }
    s_backend = LOCATIOND_BACKEND_NONE;
    s_auth_state = LOCATIOND_AUTH_NOT_DETERMINED;
}
