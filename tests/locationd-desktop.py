#!/usr/bin/env python3
# Copyright (C) 2026 Darling developers
#
# Darling is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Darling is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Darling.  If not, see <http://www.gnu.org/licenses/>.
#
# Host test: Location services bridge over D-Bus via org.freedesktop.portal.Location
# and org.freedesktop.GeoClue2 fallback for Darling.
# usage: python3 tests/locationd-desktop.py [REPO_ROOT]

import os
import shutil
import subprocess
import sys
import tempfile
import time

root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
locationd_dir = os.path.join(root, "src/locationd")
loader = os.path.join(root, "src/libdarlingdbus")

for tool in ("cc", "pkg-config", "dbus-daemon"):
    if shutil.which(tool) is None:
        print(f"SKIP: {tool} is not installed")
        sys.exit(0)

PROBE = r"""
#include "locationd_dbus.h"
#include <dbus/dbus.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static int failures = 0;

#define CHECK(cond) do { \
    if (cond) { \
        printf("PASS %d: %s\n", __LINE__, #cond); \
    } else { \
        printf("FAIL %d: %s\n", __LINE__, #cond); \
        failures++; \
    } \
} while (0)

static int g_fix_count = 0;
static struct locationd_fix g_last_fix;
static int g_auth_count = 0;
static enum locationd_auth_state g_last_auth = LOCATIOND_AUTH_NOT_DETERMINED;
static int g_error_count = 0;
static int g_last_error = 0;

static void test_on_fix(const struct locationd_fix* fix, void* context)
{
    g_fix_count++;
    g_last_fix = *fix;
}

static void test_on_auth(enum locationd_auth_state status, void* context)
{
    g_auth_count++;
    g_last_auth = status;
}

static void test_on_error(int error_code, void* context)
{
    g_error_count++;
    g_last_error = error_code;
}

// Mock Server implementation
struct mock_server {
    pid_t pid;
    int control_fd;
};

static struct mock_server* start_mock_server(const char* bus_address, int mode)
{
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0)
        return NULL;

    pid_t pid = fork();
    if (pid < 0) {
        close(fds[0]);
        close(fds[1]);
        return NULL;
    }

    if (pid == 0) {
        close(fds[0]);
        int ctrl = fds[1];

        DBusError err;
        dbus_error_init(&err);
        DBusConnection* conn = dbus_connection_open_private(bus_address, &err);
        if (!conn || !dbus_bus_register(conn, &err))
            _exit(1);

        if (mode == 1) { // Portal mode
            dbus_bus_request_name(conn, "org.freedesktop.portal.Desktop",
                                  DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
        } else if (mode == 2 || mode == 3) { // GeoClue mode (2=normal, 3=denied)
            dbus_bus_request_name(conn, "org.freedesktop.GeoClue2",
                                  DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
        }

        // Notify parent that server is ready
        char ok = '1';
        write(ctrl, &ok, 1);

        char session_path[128] = "/org/freedesktop/portal/desktop/session/1/darling_loc";
        char request_path[128] = "/org/freedesktop/portal/desktop/request/1/darling_loc";
        char geoclue_client[128] = "/org/freedesktop/GeoClue2/Client/1";
        char geoclue_loc[128] = "/org/freedesktop/GeoClue2/Location/1";

        int running = 1;
        while (running) {
            dbus_connection_read_write(conn, 50);
            DBusMessage* msg = dbus_connection_pop_message(conn);
            if (!msg) {
                // Check if control command arrived
                fd_set rfds;
                struct timeval tv = {0, 10000};
                FD_ZERO(&rfds);
                FD_SET(ctrl, &rfds);
                if (select(ctrl + 1, &rfds, NULL, NULL, &tv) > 0) {
                    char cmd = 0;
                    if (read(ctrl, &cmd, 1) <= 0) break;
                    if (cmd == 'Q') break;
                    if (cmd == 'F') { // Send portal location update
                        DBusMessage* sig = dbus_message_new_signal(
                            "/org/freedesktop/portal/desktop",
                            "org.freedesktop.portal.Location", "LocationUpdated");
                        DBusMessageIter iter, dict, entry, var;
                        dbus_message_iter_init_append(sig, &iter);
                        const char* sp = session_path;
                        dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &sp);

                        dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict);

                        // Latitude
                        const char* kLat = "Latitude";
                        double vLat = 50.0875;
                        dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &kLat);
                        dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "d", &var);
                        dbus_message_iter_append_basic(&var, DBUS_TYPE_DOUBLE, &vLat);
                        dbus_message_iter_close_container(&entry, &var);
                        dbus_message_iter_close_container(&dict, &entry);

                        // Longitude
                        const char* kLon = "Longitude";
                        double vLon = 14.4214;
                        dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &kLon);
                        dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "d", &var);
                        dbus_message_iter_append_basic(&var, DBUS_TYPE_DOUBLE, &vLon);
                        dbus_message_iter_close_container(&entry, &var);
                        dbus_message_iter_close_container(&dict, &entry);

                        // Accuracy
                        const char* kAcc = "Accuracy";
                        double vAcc = 10.0;
                        dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &kAcc);
                        dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "d", &var);
                        dbus_message_iter_append_basic(&var, DBUS_TYPE_DOUBLE, &vAcc);
                        dbus_message_iter_close_container(&entry, &var);
                        dbus_message_iter_close_container(&dict, &entry);

                        // Altitude
                        const char* kAlt = "Altitude";
                        double vAlt = 250.0;
                        dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &kAlt);
                        dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "d", &var);
                        dbus_message_iter_append_basic(&var, DBUS_TYPE_DOUBLE, &vAlt);
                        dbus_message_iter_close_container(&entry, &var);
                        dbus_message_iter_close_container(&dict, &entry);

                        // Speed
                        const char* kSpd = "Speed";
                        double vSpd = 15.0;
                        dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &kSpd);
                        dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "d", &var);
                        dbus_message_iter_append_basic(&var, DBUS_TYPE_DOUBLE, &vSpd);
                        dbus_message_iter_close_container(&entry, &var);
                        dbus_message_iter_close_container(&dict, &entry);

                        // Heading
                        const char* kHd = "Heading";
                        double vHd = 90.0;
                        dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &kHd);
                        dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "d", &var);
                        dbus_message_iter_append_basic(&var, DBUS_TYPE_DOUBLE, &vHd);
                        dbus_message_iter_close_container(&entry, &var);
                        dbus_message_iter_close_container(&dict, &entry);

                        // Timestamp (tt)
                        const char* kTs = "Timestamp";
                        dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &kTs);
                        dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "(tt)", &var);
                        DBusMessageIter st;
                        dbus_message_iter_open_container(&var, DBUS_TYPE_STRUCT, NULL, &st);
                        uint64_t sec = 1700000000;
                        uint64_t usec = 500000;
                        dbus_message_iter_append_basic(&st, DBUS_TYPE_UINT64, &sec);
                        dbus_message_iter_append_basic(&st, DBUS_TYPE_UINT64, &usec);
                        dbus_message_iter_close_container(&var, &st);
                        dbus_message_iter_close_container(&entry, &var);
                        dbus_message_iter_close_container(&dict, &entry);

                        dbus_message_iter_close_container(&iter, &dict);

                        dbus_connection_send(conn, sig, NULL);
                        dbus_connection_flush(conn);
                        dbus_message_unref(sig);
                    }
                    if (cmd == 'D') { // Deny portal request
                        DBusMessage* sig = dbus_message_new_signal(
                            request_path, "org.freedesktop.portal.Request", "Response");
                        DBusMessageIter iter, dict;
                        dbus_message_iter_init_append(sig, &iter);
                        uint32_t resp = 1; // user cancelled
                        dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &resp);
                        dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict);
                        dbus_message_iter_close_container(&iter, &dict);
                        dbus_connection_send(conn, sig, NULL);
                        dbus_connection_flush(conn);
                        dbus_message_unref(sig);
                    }
                    if (cmd == 'G') { // GeoClue emit LocationUpdated
                        DBusMessage* sig = dbus_message_new_signal(
                            geoclue_client, "org.freedesktop.GeoClue2.Client", "LocationUpdated");
                        DBusMessageIter iter;
                        dbus_message_iter_init_append(sig, &iter);
                        const char* old_p = "/";
                        const char* new_p = geoclue_loc;
                        dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &old_p);
                        dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &new_p);
                        dbus_connection_send(conn, sig, NULL);
                        dbus_connection_flush(conn);
                        dbus_message_unref(sig);
                    }
                }
                continue;
            }

            if (dbus_message_is_method_call(msg, "org.freedesktop.DBus.Peer", "Ping")) {
                DBusMessage* rep = dbus_message_new_method_return(msg);
                dbus_connection_send(conn, rep, NULL);
                dbus_connection_flush(conn);
                dbus_message_unref(rep);
            } else if (dbus_message_is_method_call(msg, "org.freedesktop.portal.Location", "CreateSession")) {
                DBusMessage* rep = dbus_message_new_method_return(msg);
                DBusMessageIter iter;
                dbus_message_iter_init_append(rep, &iter);
                const char* sp = session_path;
                dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &sp);
                dbus_connection_send(conn, rep, NULL);
                dbus_connection_flush(conn);
                dbus_message_unref(rep);
            } else if (dbus_message_is_method_call(msg, "org.freedesktop.portal.Location", "Start")) {
                DBusMessage* rep = dbus_message_new_method_return(msg);
                DBusMessageIter iter;
                dbus_message_iter_init_append(rep, &iter);
                const char* rp = request_path;
                dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &rp);
                dbus_connection_send(conn, rep, NULL);
                dbus_connection_flush(conn);
                dbus_message_unref(rep);
            } else if (dbus_message_is_method_call(msg, "org.freedesktop.portal.Session", "Close")) {
                DBusMessage* rep = dbus_message_new_method_return(msg);
                dbus_connection_send(conn, rep, NULL);
                dbus_connection_flush(conn);
                dbus_message_unref(rep);
            } else if (dbus_message_is_method_call(msg, "org.freedesktop.GeoClue2.Manager", "GetClient")) {
                DBusMessage* rep = dbus_message_new_method_return(msg);
                DBusMessageIter iter;
                dbus_message_iter_init_append(rep, &iter);
                const char* cp = geoclue_client;
                dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &cp);
                dbus_connection_send(conn, rep, NULL);
                dbus_connection_flush(conn);
                dbus_message_unref(rep);
            } else if (dbus_message_is_method_call(msg, "org.freedesktop.DBus.Properties", "Set")) {
                DBusMessage* rep = dbus_message_new_method_return(msg);
                dbus_connection_send(conn, rep, NULL);
                dbus_connection_flush(conn);
                dbus_message_unref(rep);
            } else if (dbus_message_is_method_call(msg, "org.freedesktop.GeoClue2.Client", "Start")) {
                if (mode == 3) { // GeoClue Denied mode
                    DBusMessage* err_rep = dbus_message_new_error(
                        msg, "org.freedesktop.DBus.Error.AccessDenied", "Geolocation access denied");
                    dbus_connection_send(conn, err_rep, NULL);
                    dbus_connection_flush(conn);
                    dbus_message_unref(err_rep);
                } else {
                    DBusMessage* rep = dbus_message_new_method_return(msg);
                    dbus_connection_send(conn, rep, NULL);
                    dbus_connection_flush(conn);
                    dbus_message_unref(rep);
                }
            } else if (dbus_message_is_method_call(msg, "org.freedesktop.GeoClue2.Client", "Stop") ||
                       dbus_message_is_method_call(msg, "org.freedesktop.GeoClue2.Manager", "DeleteClient")) {
                DBusMessage* rep = dbus_message_new_method_return(msg);
                dbus_connection_send(conn, rep, NULL);
                dbus_connection_flush(conn);
                dbus_message_unref(rep);
            } else if (dbus_message_is_method_call(msg, "org.freedesktop.DBus.Properties", "GetAll")) {
                DBusMessage* rep = dbus_message_new_method_return(msg);
                DBusMessageIter iter, dict, entry, var;
                dbus_message_iter_init_append(rep, &iter);
                dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict);

                const char* kLat = "Latitude";
                double vLat = 51.5074;
                dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &kLat);
                dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "d", &var);
                dbus_message_iter_append_basic(&var, DBUS_TYPE_DOUBLE, &vLat);
                dbus_message_iter_close_container(&entry, &var);
                dbus_message_iter_close_container(&dict, &entry);

                const char* kLon = "Longitude";
                double vLon = -0.1278;
                dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &kLon);
                dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "d", &var);
                dbus_message_iter_append_basic(&var, DBUS_TYPE_DOUBLE, &vLon);
                dbus_message_iter_close_container(&entry, &var);
                dbus_message_iter_close_container(&dict, &entry);

                const char* kAcc = "Accuracy";
                double vAcc = 15.0;
                dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
                dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &kAcc);
                dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "d", &var);
                dbus_message_iter_append_basic(&var, DBUS_TYPE_DOUBLE, &vAcc);
                dbus_message_iter_close_container(&entry, &var);
                dbus_message_iter_close_container(&dict, &entry);

                dbus_message_iter_close_container(&iter, &dict);
                dbus_connection_send(conn, rep, NULL);
                dbus_connection_flush(conn);
                dbus_message_unref(rep);
            }

            dbus_message_unref(msg);
        }

        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        close(ctrl);
        _exit(0);
    }

    close(fds[1]);
    struct mock_server* s = (struct mock_server*)malloc(sizeof(struct mock_server));
    s->pid = pid;
    s->control_fd = fds[0];

    // Wait for server ready signal
    char ok = 0;
    read(s->control_fd, &ok, 1);
    return s;
}

static void stop_mock_server(struct mock_server* s)
{
    if (!s) return;
    char cmd = 'Q';
    write(s->control_fd, &cmd, 1);
    close(s->control_fd);
    int status;
    waitpid(s->pid, &status, 0);
    free(s);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <session_bus_address>\n", argv[0]);
        return 1;
    }

    const char* bus_address = argv[1];

    /* Test 1: No services running */
    printf("=== Test 1: No host location service available ===\n");
    int be = locationd_dbus_init(bus_address, bus_address);
    CHECK(be == LOCATIOND_BACKEND_NONE);
    CHECK(locationd_dbus_is_available() == 0);
    locationd_dbus_close();

    /* Test 2: Portal service running, successful session and location update */
    printf("=== Test 2: Portal service location update ===\n");
    struct mock_server* portal_server = start_mock_server(bus_address, 1);
    CHECK(portal_server != NULL);

    be = locationd_dbus_init(bus_address, NULL);
    CHECK(be == LOCATIOND_BACKEND_PORTAL);
    CHECK(locationd_dbus_is_available() == 1);
    CHECK(locationd_dbus_get_backend() == LOCATIOND_BACKEND_PORTAL);

    g_fix_count = 0;
    g_auth_count = 0;
    int rc = locationd_dbus_start(-1.0, 0.0, test_on_fix, test_on_auth, test_on_error, NULL);
    CHECK(rc == 0);

    // Trigger mock server to emit LocationUpdated
    char cmd = 'F';
    write(portal_server->control_fd, &cmd, 1);

    // Process D-Bus messages in locationd_dbus
    for (int i = 0; i < 20 && g_fix_count == 0; i++) {
        locationd_dbus_process_messages(50);
    }

    CHECK(g_fix_count == 1);
    CHECK(fabs(g_last_fix.latitude - 50.0875) < 0.0001);
    CHECK(fabs(g_last_fix.longitude - 14.4214) < 0.0001);
    CHECK(fabs(g_last_fix.horizontal_accuracy - 10.0) < 0.0001);
    CHECK(fabs(g_last_fix.altitude - 250.0) < 0.0001);
    CHECK(g_last_fix.has_altitude == 1);
    CHECK(fabs(g_last_fix.speed - 15.0) < 0.0001);
    CHECK(g_last_fix.has_speed == 1);
    CHECK(fabs(g_last_fix.course - 90.0) < 0.0001);
    CHECK(g_last_fix.has_course == 1);
    CHECK(g_last_fix.timestamp >= 1700000000.0);
    CHECK(g_last_auth == LOCATIOND_AUTH_AUTHORIZED);

    /* Test 3: Portal permission denial */
    printf("=== Test 3: Portal permission denial ===\n");
    g_error_count = 0;
    g_auth_count = 0;
    cmd = 'D';
    write(portal_server->control_fd, &cmd, 1);

    for (int i = 0; i < 20 && g_error_count == 0; i++) {
        locationd_dbus_process_messages(50);
    }

    CHECK(g_error_count == 1);
    CHECK(g_last_error == 1); // kCLErrorDenied = 1
    CHECK(g_last_auth == LOCATIOND_AUTH_DENIED);

    locationd_dbus_stop();
    stop_mock_server(portal_server);
    locationd_dbus_close();

    /* Test 4: GeoClue2 fallback when portal is absent */
    printf("=== Test 4: GeoClue2 fallback ===\n");
    struct mock_server* geoclue_server = start_mock_server(bus_address, 2);
    CHECK(geoclue_server != NULL);

    // Pass invalid address for session, valid bus_address for system
    be = locationd_dbus_init("unix:path=/tmp/nonexistent_bus_path_test", bus_address);
    CHECK(be == LOCATIOND_BACKEND_GEOCLUE);
    CHECK(locationd_dbus_is_available() == 1);
    CHECK(locationd_dbus_get_backend() == LOCATIOND_BACKEND_GEOCLUE);

    g_fix_count = 0;
    g_auth_count = 0;
    rc = locationd_dbus_start(-1.0, 5.0, test_on_fix, test_on_auth, test_on_error, NULL);
    CHECK(rc == 0);

    // Trigger mock server to emit LocationUpdated
    cmd = 'G';
    write(geoclue_server->control_fd, &cmd, 1);

    for (int i = 0; i < 20 && g_fix_count == 0; i++) {
        locationd_dbus_process_messages(50);
    }

    CHECK(g_fix_count == 1);
    CHECK(fabs(g_last_fix.latitude - 51.5074) < 0.0001);
    CHECK(fabs(g_last_fix.longitude - (-0.1278)) < 0.0001);
    CHECK(fabs(g_last_fix.horizontal_accuracy - 15.0) < 0.0001);
    CHECK(g_last_auth == LOCATIOND_AUTH_AUTHORIZED);

    locationd_dbus_stop();
    stop_mock_server(geoclue_server);
    locationd_dbus_close();

    /* Test 5: GeoClue2 permission denial */
    printf("=== Test 5: GeoClue2 permission denial ===\n");
    struct mock_server* geoclue_denied_server = start_mock_server(bus_address, 3);
    CHECK(geoclue_denied_server != NULL);

    be = locationd_dbus_init("unix:path=/tmp/nonexistent_bus_path_test", bus_address);
    CHECK(be == LOCATIOND_BACKEND_GEOCLUE);

    g_error_count = 0;
    g_auth_count = 0;
    rc = locationd_dbus_start(-1.0, 0.0, test_on_fix, test_on_auth, test_on_error, NULL);
    CHECK(rc != 0); // Start failed
    CHECK(g_last_auth == LOCATIOND_AUTH_DENIED);
    CHECK(g_error_count == 1);
    CHECK(g_last_error == 1); // kCLErrorDenied = 1

    locationd_dbus_stop();
    stop_mock_server(geoclue_denied_server);
    locationd_dbus_close();

    if (failures == 0)
        printf("ALL PASSED\n");
    else
        printf("FAILED %d\n", failures);

    return failures ? 1 : 0;
}
"""

CONFIG = """<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <type>session</type>
  <listen>{address}</listen>
  <policy context="default">
    <allow send_destination="*" eavesdrop="true"/>
    <allow eavesdrop="true"/>
    <allow own="*"/>
  </policy>
</busconfig>
"""

with tempfile.TemporaryDirectory(prefix="locationd-dbus-") as tmp:
    bus_dir = os.path.join(tmp, "run_dir")
    os.mkdir(bus_dir, 0o700)
    socket_path = os.path.join(bus_dir, "bus")

    probe_c = os.path.join(tmp, "probe.c")
    with open(probe_c, "w") as f:
        f.write(PROBE)

    flags = subprocess.run(["pkg-config", "--cflags", "--libs", "dbus-1"], capture_output=True, text=True)
    if flags.returncode != 0:
        print("SKIP: libdbus-1 development files are not installed")
        sys.exit(0)

    build = subprocess.run(["cc", "-std=gnu11", "-Wall", "-g", "-fsanitize=address,undefined",
                            "-fno-sanitize-recover=undefined", f"-I{locationd_dir}", f"-I{loader}/include", probe_c,
                            os.path.join(locationd_dir, "locationd_dbus.c"), f"{loader}/darling_dbus.c",
                            *flags.stdout.split(), "-lm", "-ldl", "-lpthread", "-o", os.path.join(tmp, "probe")],
                           capture_output=True, text=True)
    if build.returncode != 0:
        print(build.stdout + build.stderr)
        sys.exit("FAIL: the probe does not build")

    config = os.path.join(tmp, "bus.conf")
    with open(config, "w") as f:
        escaped = socket_path.replace("&", "&amp;").replace("<", "&lt;")
        f.write(CONFIG.format(address="unix:path=" + escaped.replace(" ", "%20")))

    daemon = subprocess.Popen(["dbus-daemon", f"--config-file={config}", "--nofork", "--nopidfile",
                               "--print-address"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    try:
        address = daemon.stdout.readline().strip()
        if not address:
            print(daemon.stderr.read())
            sys.exit("FAIL: the private dbus-daemon did not start")
        address = address.split(",guid=")[0]
        env = dict(os.environ)
        env["ASAN_OPTIONS"] = "detect_leaks=0"
        run = subprocess.run([os.path.join(tmp, "probe"), address], capture_output=True, text=True,
                             env=env, timeout=60)
        sys.stdout.write(run.stdout + run.stderr)
        if run.returncode != 0:
            sys.exit("FAIL: location services desktop bridge test failed")
    finally:
        daemon.terminate()
        daemon.wait()

print("PASS: CoreLocation locationd host bridge via xdg-desktop-portal and GeoClue2 verified")
