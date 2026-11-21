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
# Host test: Desktop appearance settings bridge over D-Bus via org.freedesktop.portal.Settings
# and config fallbacks for Darling.
# usage: python3 tests/settingsd-desktop-appearance.py [REPO_ROOT]
#
# settingsd_dbus.c is built natively against the host's libdbus-1 with ASan
# and UBSan. A private dbus-daemon and a fake portal server respond to
# ReadOne/Read method calls and emit SettingChanged signals from the portal.

import os
import shutil
import subprocess
import sys
import tempfile
import time

root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
settingsd = os.path.join(root, "src/settingsd")
loader = os.path.join(root, "src/libdarlingdbus")

for tool in ("cc", "pkg-config", "dbus-daemon"):
    if shutil.which(tool) is None:
        print(f"SKIP: {tool} is not installed")
        sys.exit(0)

PROBE = r"""
#include "settingsd_dbus.h"
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

static const char *bus_address;
static int failures;

#define CHECK(cond) do { \
    if (cond) { \
        printf("PASS %d: %s\n", __LINE__, #cond); \
    } else { \
        printf("FAIL %d: %s\n", __LINE__, #cond); \
        failures++; \
    } \
} while (0)

/* ---- Fake portal server ---- */
static int s_color_scheme = 1; // 1 = Dark
static int s_contrast = 1;     // 1 = High contrast
static double s_accent_r = 0.686, s_accent_g = 0.322, s_accent_b = 0.871; // Purple
static int s_readone_supported = 1;

static void emit_portal_setting_changed_color(DBusConnection *c, dbus_uint32_t val)
{
    DBusMessage *sig = dbus_message_new_signal(
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.Settings",
        "SettingChanged");
    DBusMessageIter it, v1;
    dbus_message_iter_init_append(sig, &it);
    const char *ns = "org.freedesktop.appearance";
    const char *key = "color-scheme";
    dbus_message_iter_append_basic(&it, DBUS_TYPE_STRING, &ns);
    dbus_message_iter_append_basic(&it, DBUS_TYPE_STRING, &key);
    dbus_message_iter_open_container(&it, DBUS_TYPE_VARIANT, "u", &v1);
    dbus_message_iter_append_basic(&v1, DBUS_TYPE_UINT32, &val);
    dbus_message_iter_close_container(&it, &v1);
    dbus_connection_send(c, sig, NULL);
    dbus_connection_flush(c);
    dbus_message_unref(sig);
}

static void emit_portal_setting_changed_accent(DBusConnection *c, double r, double g, double b)
{
    DBusMessage *sig = dbus_message_new_signal(
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.Settings",
        "SettingChanged");
    DBusMessageIter it, v1, s_it;
    dbus_message_iter_init_append(sig, &it);
    const char *ns = "org.freedesktop.appearance";
    const char *key = "accent-color";
    dbus_message_iter_append_basic(&it, DBUS_TYPE_STRING, &ns);
    dbus_message_iter_append_basic(&it, DBUS_TYPE_STRING, &key);
    dbus_message_iter_open_container(&it, DBUS_TYPE_VARIANT, "(ddd)", &v1);
    dbus_message_iter_open_container(&v1, DBUS_TYPE_STRUCT, NULL, &s_it);
    dbus_message_iter_append_basic(&s_it, DBUS_TYPE_DOUBLE, &r);
    dbus_message_iter_append_basic(&s_it, DBUS_TYPE_DOUBLE, &g);
    dbus_message_iter_append_basic(&s_it, DBUS_TYPE_DOUBLE, &b);
    dbus_message_iter_close_container(&v1, &s_it);
    dbus_message_iter_close_container(&it, &v1);
    dbus_connection_send(c, sig, NULL);
    dbus_connection_flush(c);
    dbus_message_unref(sig);
}

static void serve_server(int ready_fd)
{
    DBusError error;
    dbus_error_init(&error);
    DBusConnection *c = dbus_connection_open_private(bus_address, &error);
    if (!c || !dbus_bus_register(c, &error)
        || dbus_bus_request_name(c, "org.freedesktop.portal.Desktop", DBUS_NAME_FLAG_DO_NOT_QUEUE, &error)
            != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
        _exit(3);

    if (write(ready_fd, "r", 1) != 1)
        _exit(3);
    close(ready_fd);

    while (dbus_connection_read_write(c, -1))
    {
        DBusMessage *m;
        while ((m = dbus_connection_pop_message(c)) != NULL)
        {
            if (dbus_message_is_method_call(m, "org.test.Trigger", "SetColorScheme"))
            {
                DBusMessageIter it;
                dbus_message_iter_init(m, &it);
                dbus_uint32_t val = 0;
                if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_UINT32) {
                    dbus_message_iter_get_basic(&it, &val);
                    s_color_scheme = (int)val;
                    emit_portal_setting_changed_color(c, val);
                }
                DBusMessage *rep = dbus_message_new_method_return(m);
                dbus_connection_send(c, rep, NULL);
                dbus_connection_flush(c);
                dbus_message_unref(rep);
            }
            else if (dbus_message_is_method_call(m, "org.test.Trigger", "SetAccentColor"))
            {
                DBusMessageIter it;
                dbus_message_iter_init(m, &it);
                double r = 0, g = 0, b = 0;
                dbus_message_iter_get_basic(&it, &r);
                dbus_message_iter_next(&it);
                dbus_message_iter_get_basic(&it, &g);
                dbus_message_iter_next(&it);
                dbus_message_iter_get_basic(&it, &b);
                s_accent_r = r; s_accent_g = g; s_accent_b = b;
                emit_portal_setting_changed_accent(c, r, g, b);

                DBusMessage *rep = dbus_message_new_method_return(m);
                dbus_connection_send(c, rep, NULL);
                dbus_connection_flush(c);
                dbus_message_unref(rep);
            }
            else if (dbus_message_is_method_call(m, "org.test.Trigger", "DisableReadOne"))
            {
                s_readone_supported = 0;
                DBusMessage *rep = dbus_message_new_method_return(m);
                dbus_connection_send(c, rep, NULL);
                dbus_connection_flush(c);
                dbus_message_unref(rep);
            }
            else if (dbus_message_is_method_call(m, "org.freedesktop.portal.Settings", "ReadOne")
                     || dbus_message_is_method_call(m, "org.freedesktop.portal.Settings", "Read"))
            {
                int is_readone = dbus_message_is_method_call(m, "org.freedesktop.portal.Settings", "ReadOne");
                if (is_readone && !s_readone_supported)
                {
                    DBusMessage *err_rep = dbus_message_new_error(m,
                        "org.freedesktop.DBus.Error.UnknownMethod", "ReadOne is not implemented");
                    dbus_connection_send(c, err_rep, NULL);
                    dbus_connection_flush(c);
                    dbus_message_unref(err_rep);
                    dbus_message_unref(m);
                    continue;
                }

                DBusMessageIter it;
                dbus_message_iter_init(m, &it);
                const char *ns = NULL, *key = NULL;
                if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_STRING)
                {
                    dbus_message_iter_get_basic(&it, &ns);
                    dbus_message_iter_next(&it);
                    if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_STRING)
                        dbus_message_iter_get_basic(&it, &key);
                }

                DBusMessage *reply = NULL;
                if (!key || strcmp(ns, "org.freedesktop.appearance") != 0
                    || (strcmp(key, "color-scheme") != 0 && strcmp(key, "contrast") != 0 && strcmp(key, "accent-color") != 0))
                {
                    reply = dbus_message_new_error(m,
                        "org.freedesktop.portal.Error.NotFound", "The requested key was not found");
                }
                else
                {
                    reply = dbus_message_new_method_return(m);
                    DBusMessageIter ret_it, v1, v2;
                    dbus_message_iter_init_append(reply, &ret_it);

                    if (strcmp(key, "color-scheme") == 0)
                    {
                        dbus_message_iter_open_container(&ret_it, DBUS_TYPE_VARIANT, "v", &v1);
                        dbus_message_iter_open_container(&v1, DBUS_TYPE_VARIANT, "u", &v2);
                        dbus_uint32_t val = (dbus_uint32_t) s_color_scheme;
                        dbus_message_iter_append_basic(&v2, DBUS_TYPE_UINT32, &val);
                        dbus_message_iter_close_container(&v1, &v2);
                        dbus_message_iter_close_container(&ret_it, &v1);
                    }
                    else if (strcmp(key, "contrast") == 0)
                    {
                        dbus_message_iter_open_container(&ret_it, DBUS_TYPE_VARIANT, "v", &v1);
                        dbus_message_iter_open_container(&v1, DBUS_TYPE_VARIANT, "u", &v2);
                        dbus_uint32_t val = (dbus_uint32_t) s_contrast;
                        dbus_message_iter_append_basic(&v2, DBUS_TYPE_UINT32, &val);
                        dbus_message_iter_close_container(&v1, &v2);
                        dbus_message_iter_close_container(&ret_it, &v1);
                    }
                    else if (strcmp(key, "accent-color") == 0)
                    {
                        dbus_message_iter_open_container(&ret_it, DBUS_TYPE_VARIANT, "v", &v1);
                        dbus_message_iter_open_container(&v1, DBUS_TYPE_VARIANT, "(ddd)", &v2);
                        DBusMessageIter s_it;
                        dbus_message_iter_open_container(&v2, DBUS_TYPE_STRUCT, NULL, &s_it);
                        dbus_message_iter_append_basic(&s_it, DBUS_TYPE_DOUBLE, &s_accent_r);
                        dbus_message_iter_append_basic(&s_it, DBUS_TYPE_DOUBLE, &s_accent_g);
                        dbus_message_iter_append_basic(&s_it, DBUS_TYPE_DOUBLE, &s_accent_b);
                        dbus_message_iter_close_container(&v2, &s_it);
                        dbus_message_iter_close_container(&v1, &v2);
                        dbus_message_iter_close_container(&ret_it, &v1);
                    }
                }

                dbus_connection_send(c, reply, NULL);
                dbus_connection_flush(c);
                dbus_message_unref(reply);
            }
            dbus_message_unref(m);
        }
    }
    _exit(0);
}

static pid_t start_server(void)
{
    int fds[2];
    if (pipe(fds) != 0) return -1;
    pid_t pid = fork();
    if (pid == 0)
    {
        close(fds[0]);
        serve_server(fds[1]);
    }
    close(fds[1]);
    char b;
    if (read(fds[0], &b, 1) != 1)
    {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        close(fds[0]);
        return -1;
    }
    close(fds[0]);
    return pid;
}

static void stop_server(pid_t pid)
{
    if (pid > 0)
    {
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
    }
}

static int g_cb_calls = 0;
static int g_last_has_dark = 0;
static int g_last_dark = -1;
static int g_last_has_contrast = 0;
static int g_last_contrast = -1;
static int g_last_has_accent = 0;
static double g_last_r = 0, g_last_g = 0, g_last_b = 0;

static void test_cb(int has_dark, int dark, int has_contrast, int contrast, int has_accent, double r, double g, double b)
{
    g_cb_calls++;
    g_last_has_dark = has_dark;
    g_last_dark = dark;
    g_last_has_contrast = has_contrast;
    g_last_contrast = contrast;
    g_last_has_accent = has_accent;
    g_last_r = r;
    g_last_g = g;
    g_last_b = b;
}

int main(int argc, char *argv[])
{
    if (argc < 2) return 1;
    bus_address = argv[1];

    setenv("DBUS_SESSION_BUS_ADDRESS", bus_address, 1);

    pid_t server = start_server();
    CHECK(server > 0);

    /* Test 1: Initialize client */
    int ret = settingsd_dbus_init();
    CHECK(ret == 0);

    /* Test 2: Verify socket file descriptor */
    int fd = -1;
    ret = settingsd_dbus_get_fd(&fd);
    CHECK(ret == 0);
    CHECK(fd >= 0);

    /* Test 3: Read appearance from portal using ReadOne */
    int has_dark = 0, dark = 0;
    int has_contrast = 0, contrast = 0;
    int has_accent = 0;
    double r = 0, g = 0, b = 0;
    ret = settingsd_dbus_read_appearance(&has_dark, &dark, &has_contrast, &contrast, &has_accent, &r, &g, &b);
    CHECK(ret == 0);
    CHECK(has_dark == 1);
    CHECK(dark == 1);
    CHECK(has_contrast == 1);
    CHECK(contrast == 1);
    CHECK(has_accent == 1);
    CHECK(fabs(r - 0.686) < 0.01);
    CHECK(fabs(g - 0.322) < 0.01);
    CHECK(fabs(b - 0.871) < 0.01);

    /* Test 4: Disable ReadOne on server to test fallback to Read */
    DBusError err;
    dbus_error_init(&err);
    DBusConnection *ctrl_conn = dbus_connection_open_private(bus_address, &err);
    CHECK(ctrl_conn != NULL);
    CHECK(dbus_bus_register(ctrl_conn, &err));

    DBusMessage *ctrl_msg = dbus_message_new_method_call(
        "org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.test.Trigger", "DisableReadOne");
    DBusMessage *ctrl_rep = dbus_connection_send_with_reply_and_block(ctrl_conn, ctrl_msg, 5000, &err);
    CHECK(ctrl_rep != NULL);
    dbus_message_unref(ctrl_msg);
    dbus_message_unref(ctrl_rep);

    /* Re-read appearance to verify Read fallback works seamlessly */
    has_dark = dark = has_contrast = contrast = has_accent = 0;
    ret = settingsd_dbus_read_appearance(&has_dark, &dark, &has_contrast, &contrast, &has_accent, &r, &g, &b);
    CHECK(ret == 0);
    CHECK(has_dark == 1 && dark == 1);
    CHECK(has_accent == 1);

    /* Test 5: SettingChanged signal from portal for color-scheme */
    ctrl_msg = dbus_message_new_method_call(
        "org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.test.Trigger", "SetColorScheme");
    DBusMessageIter it;
    dbus_message_iter_init_append(ctrl_msg, &it);
    dbus_uint32_t new_color = 0; // Light
    dbus_message_iter_append_basic(&it, DBUS_TYPE_UINT32, &new_color);
    ctrl_rep = dbus_connection_send_with_reply_and_block(ctrl_conn, ctrl_msg, 5000, &err);
    CHECK(ctrl_rep != NULL);
    dbus_message_unref(ctrl_msg);
    dbus_message_unref(ctrl_rep);

    settingsd_dbus_process_messages(100, test_cb);
    CHECK(g_cb_calls == 1);
    CHECK(g_last_has_dark == 1);
    CHECK(g_last_dark == 0);

    /* Test 6: SettingChanged signal for valid accent-color */
    ctrl_msg = dbus_message_new_method_call(
        "org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.test.Trigger", "SetAccentColor");
    dbus_message_iter_init_append(ctrl_msg, &it);
    double orange_r = 1.0, orange_g = 0.584, orange_b = 0.0;
    dbus_message_iter_append_basic(&it, DBUS_TYPE_DOUBLE, &orange_r);
    dbus_message_iter_append_basic(&it, DBUS_TYPE_DOUBLE, &orange_g);
    dbus_message_iter_append_basic(&it, DBUS_TYPE_DOUBLE, &orange_b);
    ctrl_rep = dbus_connection_send_with_reply_and_block(ctrl_conn, ctrl_msg, 5000, &err);
    CHECK(ctrl_rep != NULL);
    dbus_message_unref(ctrl_msg);
    dbus_message_unref(ctrl_rep);

    settingsd_dbus_process_messages(100, test_cb);
    CHECK(g_cb_calls == 2);
    CHECK(g_last_has_accent == 1);
    CHECK(fabs(g_last_r - 1.0) < 0.01);
    CHECK(fabs(g_last_g - 0.584) < 0.01);
    CHECK(fabs(g_last_b - 0.0) < 0.01);

    /* Test 7: SettingChanged signal with out-of-range accent (must be treated as not set) */
    ctrl_msg = dbus_message_new_method_call(
        "org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.test.Trigger", "SetAccentColor");
    dbus_message_iter_init_append(ctrl_msg, &it);
    double invalid_r = 1.5, invalid_g = 0.0, invalid_b = 0.0;
    dbus_message_iter_append_basic(&it, DBUS_TYPE_DOUBLE, &invalid_r);
    dbus_message_iter_append_basic(&it, DBUS_TYPE_DOUBLE, &invalid_g);
    dbus_message_iter_append_basic(&it, DBUS_TYPE_DOUBLE, &invalid_b);
    ctrl_rep = dbus_connection_send_with_reply_and_block(ctrl_conn, ctrl_msg, 5000, &err);
    CHECK(ctrl_rep != NULL);
    dbus_message_unref(ctrl_msg);
    dbus_message_unref(ctrl_rep);

    settingsd_dbus_process_messages(100, test_cb);
    CHECK(g_cb_calls == 3);
    CHECK(g_last_has_accent == 0); // Out of bounds treated as not set!

    dbus_connection_close(ctrl_conn);
    dbus_connection_unref(ctrl_conn);

    /* Test 8: Disconnect handling */
    shutdown(fd, SHUT_RDWR);
    ret = settingsd_dbus_process_messages(50, test_cb);
    CHECK(ret < 0); // Disconnection detected cleanly

    stop_server(server);

    settingsd_dbus_close();

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

with tempfile.TemporaryDirectory(prefix="settingsd-dbus-") as tmp:
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
                            "-fno-sanitize-recover=undefined", f"-I{settingsd}", f"-I{loader}/include", probe_c,
                            os.path.join(settingsd, "settingsd_dbus.c"), f"{loader}/darling_dbus.c",
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
            sys.exit("FAIL: desktop appearance settings bridge test failed")
    finally:
        daemon.terminate()
        daemon.wait()

print("PASS: desktop appearance settings bridge via org.freedesktop.portal.Settings verified")
