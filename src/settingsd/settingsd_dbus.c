/*
 * Copyright (C) 2026 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "settingsd_dbus.h"
#include <dbus/dbus.h>
#include <darling_dbus.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORTAL_NAME "org.freedesktop.portal.Desktop"
#define PORTAL_PATH "/org/freedesktop/portal/desktop"
#define PORTAL_INTERFACE "org.freedesktop.portal.Settings"
#define SETTINGS_NAMESPACE "org.freedesktop.appearance"
#define MAX_ADDRESS_LENGTH 1024

static const struct darling_dbus *s_api = NULL;
static DBusConnection *s_conn = NULL;

static int s_has_dark = 0;
static int s_dark = 0;
static int s_has_contrast = 0;
static int s_contrast = 0;
static int s_has_accent = 0;
static double s_r = 0.0, s_g = 0.48, s_b = 1.0;

static int load_dbus_symbols(void)
{
    if (s_api == NULL)
        s_api = darling_dbus_get(DARLING_DBUS_HOST_FALLBACK);
    return s_api != NULL ? 0 : -1;
}

static int session_address_allowed(const char *address) {
    if (strncmp(address, "unix:path=", 10) != 0 && strncmp(address, "unix:abstract=", 14) != 0)
        return 0;
    if (strlen(address) >= MAX_ADDRESS_LENGTH)
        return 0;
    for (const unsigned char *c = (const unsigned char*) address; *c != '\0'; c++) {
        if (*c <= ' ' || *c >= 0x7f || *c == ';')
            return 0;
    }
    return strstr(address, "unixexec") == NULL;
}

static char *get_session_bus_address(void) {
    const char *env = getenv("DBUS_SESSION_BUS_ADDRESS");
    if (env && env[0] != '\0') {
        return session_address_allowed(env) ? strdup(env) : NULL;
    }

    const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
    if (runtime_dir && runtime_dir[0] == '/') {
        char path[MAX_ADDRESS_LENGTH];
        if (snprintf(path, sizeof(path), "%s/bus", runtime_dir) < (int)sizeof(path)) {
            if (access(path, R_OK | W_OK) == 0) {
                char *escaped = s_api->address_escape_value(path);
                if (escaped) {
                    size_t len = strlen("unix:path=") + strlen(escaped) + 1;
                    char *addr = malloc(len);
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

int settingsd_dbus_init(void) {
    if (s_conn != NULL) {
        if (s_api->connection_get_is_connected(s_conn))
            return 0;
        settingsd_dbus_close();
    }

    if (load_dbus_symbols() != 0)
        return -1;

    char *address = get_session_bus_address();
    if (!address)
        return -2;

    DBusError err;
    s_api->error_init(&err);

    s_conn = s_api->connection_open_private(address, &err);
    free(address);

    if (!s_conn) {
        s_api->error_free(&err);
        return -3;
    }

    if (!s_api->bus_register(s_conn, &err)) {
        s_api->error_free(&err);
        s_api->connection_close(s_conn);
        s_api->connection_unref(s_conn);
        s_conn = NULL;
        return -4;
    }

    // Subscribe to SettingChanged signal from the portal
    s_api->bus_add_match(s_conn,
        "type='signal',sender='" PORTAL_NAME "',path='" PORTAL_PATH "',interface='" PORTAL_INTERFACE "',member='SettingChanged'",
        &err);
    s_api->bus_add_match(s_conn,
        "type='signal',interface='org.freedesktop.DBus.Local',member='Disconnected'",
        &err);
    s_api->connection_flush(s_conn);
    s_api->error_free(&err);

    return 0;
}

int settingsd_dbus_get_fd(int *out_fd) {
    if (!s_conn || !out_fd)
        return -1;
    if (!s_api->connection_get_unix_fd(s_conn, out_fd))
        return -2;
    return 0;
}

static int unwrap_variant(DBusMessageIter *iter, DBusMessageIter *out_cur) {
    *out_cur = *iter;
    while (s_api->message_iter_get_arg_type(out_cur) == DBUS_TYPE_VARIANT) {
        DBusMessageIter sub;
        s_api->message_iter_recurse(out_cur, &sub);
        *out_cur = sub;
    }
    return 1;
}

static int extract_uint32_value(DBusMessageIter *iter, uint32_t *out) {
    DBusMessageIter cur;
    unwrap_variant(iter, &cur);
    if (s_api->message_iter_get_arg_type(&cur) == DBUS_TYPE_UINT32) {
        s_api->message_iter_get_basic(&cur, out);
        return 1;
    }
    return 0;
}

static int extract_accent_color_value(DBusMessageIter *iter, double *out_r, double *out_g, double *out_b) {
    DBusMessageIter cur;
    unwrap_variant(iter, &cur);
    if (s_api->message_iter_get_arg_type(&cur) == DBUS_TYPE_STRUCT) {
        DBusMessageIter sIter;
        s_api->message_iter_recurse(&cur, &sIter);
        if (s_api->message_iter_get_arg_type(&sIter) == DBUS_TYPE_DOUBLE) {
            double r = 0, g = 0, b = 0;
            s_api->message_iter_get_basic(&sIter, &r);
            if (s_api->message_iter_next(&sIter) && s_api->message_iter_get_arg_type(&sIter) == DBUS_TYPE_DOUBLE) {
                s_api->message_iter_get_basic(&sIter, &g);
                if (s_api->message_iter_next(&sIter) && s_api->message_iter_get_arg_type(&sIter) == DBUS_TYPE_DOUBLE) {
                    s_api->message_iter_get_basic(&sIter, &b);
                    if (r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0) {
                        *out_r = r;
                        *out_g = g;
                        *out_b = b;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

// Queries a single setting key from org.freedesktop.portal.Settings.
// Calls ReadOne first, falling back to Read if ReadOne returns UnknownMethod.
// Uses a fresh DBusError per call and unrefs reply.
static int read_portal_key(const char *key, DBusMessageIter *out_value_iter, DBusMessage **out_reply) {
    if (!s_conn)
        return 0;

    const char *methods[] = { "ReadOne", "Read" };
    for (size_t m = 0; m < sizeof(methods) / sizeof(methods[0]); m++) {
        DBusError err;
        s_api->error_init(&err);

        DBusMessage *msg = s_api->message_new_method_call(PORTAL_NAME, PORTAL_PATH, PORTAL_INTERFACE, methods[m]);
        if (!msg) {
            s_api->error_free(&err);
            return 0;
        }

        DBusMessageIter iter;
        s_api->message_iter_init_append(msg, &iter);
        const char *ns = SETTINGS_NAMESPACE;
        s_api->message_iter_append_basic(&iter, DBUS_TYPE_STRING, &ns);
        s_api->message_iter_append_basic(&iter, DBUS_TYPE_STRING, &key);

        DBusMessage *reply = s_api->connection_send_with_reply_and_block(s_conn, msg, 5000, &err);
        s_api->message_unref(msg);

        if (reply) {
            if (s_api->message_get_type(reply) == DBUS_MESSAGE_TYPE_METHOD_RETURN) {
                if (s_api->message_iter_init(reply, out_value_iter)) {
                    s_api->error_free(&err);
                    *out_reply = reply;
                    return 1;
                }
            }
            s_api->message_unref(reply);
        }

        int unknown_method = (err.name && strcmp(err.name, "org.freedesktop.DBus.Error.UnknownMethod") == 0);
        s_api->error_free(&err);

        if (!unknown_method) {
            // Error was something other than UnknownMethod (e.g. key not found or portal error)
            break;
        }
    }

    return 0;
}

int settingsd_dbus_read_appearance(int *out_has_dark, int *out_dark,
                                   int *out_has_contrast, int *out_contrast,
                                   int *out_has_accent, double *out_r, double *out_g, double *out_b) {
    if (!s_conn) {
        if (settingsd_dbus_init() != 0)
            return -1;
    }

    int read_count = 0;
    DBusMessageIter iter;
    DBusMessage *reply = NULL;

    // 1. color-scheme
    if (read_portal_key("color-scheme", &iter, &reply)) {
        uint32_t val = 0;
        if (extract_uint32_value(&iter, &val)) {
            s_has_dark = 1;
            s_dark = (val == 1) ? 1 : 0;
            read_count++;
        }
        s_api->message_unref(reply);
    }

    // 2. contrast
    if (read_portal_key("contrast", &iter, &reply)) {
        uint32_t val = 0;
        if (extract_uint32_value(&iter, &val)) {
            s_has_contrast = 1;
            s_contrast = (val == 1) ? 1 : 0;
            read_count++;
        }
        s_api->message_unref(reply);
    }

    // 3. accent-color
    if (read_portal_key("accent-color", &iter, &reply)) {
        double r = 0, g = 0, b = 0;
        if (extract_accent_color_value(&iter, &r, &g, &b)) {
            s_has_accent = 1;
            s_r = r;
            s_g = g;
            s_b = b;
            read_count++;
        } else {
            // Values outside [0, 1] treated as not set
            s_has_accent = 0;
        }
        s_api->message_unref(reply);
    }

    if (out_has_dark) *out_has_dark = s_has_dark;
    if (out_dark) *out_dark = s_dark;
    if (out_has_contrast) *out_has_contrast = s_has_contrast;
    if (out_contrast) *out_contrast = s_contrast;
    if (out_has_accent) *out_has_accent = s_has_accent;
    if (out_r) *out_r = s_r;
    if (out_g) *out_g = s_g;
    if (out_b) *out_b = s_b;

    return (read_count > 0 || s_conn != NULL) ? 0 : -1;
}

int settingsd_dbus_process_messages(int timeout_ms, settingsd_change_cb callback) {
    if (!s_conn)
        return -1;

    if (!s_api->connection_get_is_connected(s_conn)) {
        settingsd_dbus_close();
        return -1;
    }

    if (!s_api->connection_read_write(s_conn, timeout_ms)) {
        settingsd_dbus_close();
        return -1;
    }

    int processed = 0;
    int state_changed = 0;
    DBusMessage *msg;

    while ((msg = s_api->connection_pop_message(s_conn)) != NULL) {
        processed++;

        if (s_api->message_is_signal(msg, "org.freedesktop.DBus.Local", "Disconnected")) {
            s_api->message_unref(msg);
            settingsd_dbus_close();
            return -1;
        }

        if (s_api->message_is_signal(msg, PORTAL_INTERFACE, "SettingChanged")) {
            const char *path = s_api->message_get_path(msg);
            if (!path || strcmp(path, PORTAL_PATH) != 0) {
                s_api->message_unref(msg);
                continue;
            }

            DBusMessageIter iter;
            if (s_api->message_iter_init(msg, &iter)) {
                const char *ns = NULL;
                const char *key = NULL;
                if (s_api->message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING) {
                    s_api->message_iter_get_basic(&iter, &ns);
                    if (s_api->message_iter_next(&iter) && s_api->message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING) {
                        s_api->message_iter_get_basic(&iter, &key);
                        if (s_api->message_iter_next(&iter) && ns && strcmp(ns, SETTINGS_NAMESPACE) == 0 && key) {
                            if (strcmp(key, "color-scheme") == 0) {
                                uint32_t val = 0;
                                if (extract_uint32_value(&iter, &val)) {
                                    s_has_dark = 1;
                                    s_dark = (val == 1) ? 1 : 0;
                                    state_changed = 1;
                                }
                            } else if (strcmp(key, "contrast") == 0) {
                                uint32_t val = 0;
                                if (extract_uint32_value(&iter, &val)) {
                                    s_has_contrast = 1;
                                    s_contrast = (val == 1) ? 1 : 0;
                                    state_changed = 1;
                                }
                            } else if (strcmp(key, "accent-color") == 0) {
                                double r = 0, g = 0, b = 0;
                                if (extract_accent_color_value(&iter, &r, &g, &b)) {
                                    s_has_accent = 1;
                                    s_r = r;
                                    s_g = g;
                                    s_b = b;
                                    state_changed = 1;
                                } else {
                                    s_has_accent = 0;
                                    state_changed = 1;
                                }
                            }
                        }
                    }
                }
            }
        }

        s_api->message_unref(msg);
    }

    if (state_changed && callback) {
        callback(s_has_dark, s_dark, s_has_contrast, s_contrast, s_has_accent, s_r, s_g, s_b);
    }

    return processed;
}

void settingsd_dbus_close(void) {
    if (s_conn) {
        s_api->connection_close(s_conn);
        s_api->connection_unref(s_conn);
        s_conn = NULL;
    }
}
