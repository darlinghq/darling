#!/usr/bin/env python3
# Host test: UserNotifications / NSUserNotification delivery to Linux desktop
# via org.freedesktop.Notifications (and xdg-desktop-portal fallback).
# usage: python3 tests/usernotifications-desktop-delivery.py [REPO_ROOT]
#
# usernoted_dbus.c is built natively against the host's libdbus-1 with ASan
# and UBSan. A private dbus-daemon and a fake notification server handle
# notifications and emit ActionInvoked / NotificationClosed signals.

import os
import shutil
import subprocess
import sys
import tempfile
import time

root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
usernoted = os.path.join(root, "src/usernoted")

for tool in ("cc", "pkg-config", "dbus-daemon"):
    if shutil.which(tool) is None:
        print(f"SKIP: {tool} is not installed")
        sys.exit(0)

PROBE = r"""
#include "usernoted_dbus.h"
#include <dbus/dbus.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static const char *bus_dir, *bus_address;
static char record_path[4096];
static int failures;

#define CHECK(cond) do { \
	if (cond) { \
		printf("PASS %d: %s\n", __LINE__, #cond); \
	} else { \
		printf("FAIL %d: %s\n", __LINE__, #cond); \
		failures++; \
	} \
} while (0)

static const char *env_address, *env_runtime_dir;
static char *fake_getenv(const char *name)
{
	if (strcmp(name, "DBUS_SESSION_BUS_ADDRESS") == 0)
		return (char *) env_address;
	if (strcmp(name, "XDG_RUNTIME_DIR") == 0)
		return (char *) env_runtime_dir;
	return NULL;
}

static struct usernoted_dbus api = {
	dbus_connection_open_private,
	dbus_bus_register,
	dbus_bus_add_match,
	dbus_connection_close,
	dbus_connection_unref,
	dbus_connection_read_write,
	dbus_connection_pop_message,
	dbus_connection_send,
	dbus_connection_flush,
	dbus_connection_send_with_reply_and_block,
	dbus_message_new_method_call,
	dbus_message_new_method_return,
	dbus_message_new_error,
	dbus_message_iter_init_append,
	dbus_message_iter_append_basic,
	dbus_message_iter_open_container,
	dbus_message_iter_close_container,
	dbus_message_iter_init,
	dbus_message_iter_get_arg_type,
	dbus_message_iter_get_basic,
	dbus_message_iter_recurse,
	dbus_message_iter_next,
	dbus_message_is_signal,
	dbus_message_is_method_call,
	dbus_message_get_path,
	dbus_message_get_interface,
	dbus_message_get_member,
	dbus_message_get_signature,
	dbus_message_unref,
	dbus_error_init,
	dbus_error_free,
	dbus_address_escape_value,
	dbus_free,
	fake_getenv,
	0,
};

static void record(const char *line)
{
	FILE *f = fopen(record_path, "a");
	if (f) { fprintf(f, "%s\n", line); fclose(f); }
}

/* ---- Fake notification daemon server ---- */
static uint32_t s_server_next_id = 1;

static void serve_server(int ready_fd)
{
	DBusError error;
	dbus_error_init(&error);
	DBusConnection *c = dbus_connection_open_private(bus_address, &error);
	if (!c || !dbus_bus_register(c, &error)
		|| dbus_bus_request_name(c, "org.freedesktop.Notifications", DBUS_NAME_FLAG_DO_NOT_QUEUE, &error)
			!= DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER
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
			if (dbus_message_is_method_call(m, "org.freedesktop.Notifications", "Notify"))
			{
				DBusMessageIter it, act_it, hints_it;
				const char *app_name = NULL, *app_icon = NULL, *summary = NULL, *body = NULL;
				dbus_uint32_t replaces_id = 0;
				dbus_int32_t timeout = 0;
				char line[8192];
				int action_count = 0;
				int urgency = -1;
				char image_path[512] = "";
				char sound_file[512] = "";

				dbus_message_iter_init(m, &it);
				if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_STRING)
					dbus_message_iter_get_basic(&it, &app_name);
				dbus_message_iter_next(&it);
				if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_UINT32)
					dbus_message_iter_get_basic(&it, &replaces_id);
				dbus_message_iter_next(&it);
				if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_STRING)
					dbus_message_iter_get_basic(&it, &app_icon);
				dbus_message_iter_next(&it);
				if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_STRING)
					dbus_message_iter_get_basic(&it, &summary);
				dbus_message_iter_next(&it);
				if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_STRING)
					dbus_message_iter_get_basic(&it, &body);
				dbus_message_iter_next(&it);

				/* Actions */
				if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_ARRAY)
				{
					dbus_message_iter_recurse(&it, &act_it);
					while (dbus_message_iter_get_arg_type(&act_it) != DBUS_TYPE_INVALID)
					{
						action_count++;
						dbus_message_iter_next(&act_it);
					}
				}
				dbus_message_iter_next(&it);

				/* Hints */
				if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_ARRAY)
				{
					dbus_message_iter_recurse(&it, &hints_it);
					while (dbus_message_iter_get_arg_type(&hints_it) == DBUS_TYPE_DICT_ENTRY)
					{
						DBusMessageIter entry, val;
						const char *key = NULL;
						dbus_message_iter_recurse(&hints_it, &entry);
						dbus_message_iter_get_basic(&entry, &key);
						dbus_message_iter_next(&entry);
						dbus_message_iter_recurse(&entry, &val);
						if (key && strcmp(key, "urgency") == 0)
						{
							unsigned char u = 0;
							dbus_message_iter_get_basic(&val, &u);
							urgency = u;
						}
						else if (key && strcmp(key, "image-path") == 0)
						{
							const char *p = NULL;
							dbus_message_iter_get_basic(&val, &p);
							if (p) snprintf(image_path, sizeof(image_path), "%s", p);
						}
						else if (key && strcmp(key, "sound-file") == 0)
						{
							const char *s = NULL;
							dbus_message_iter_get_basic(&val, &s);
							if (s) snprintf(sound_file, sizeof(sound_file), "%s", s);
						}
						dbus_message_iter_next(&hints_it);
					}
				}
				dbus_message_iter_next(&it);
				if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_INT32)
					dbus_message_iter_get_basic(&it, &timeout);

				uint32_t assigned_id = s_server_next_id++;
				snprintf(line, sizeof(line), "Notify\t%u\t%s\t%s\t%s\t%d\t%d\t%s\t%s",
					assigned_id, app_name ? app_name : "",
					summary ? summary : "", body ? body : "",
					action_count, urgency, image_path, sound_file);
				record(line);

				DBusMessage *reply = dbus_message_new_method_return(m);
				dbus_uint32_t rid = assigned_id;
				dbus_message_append_args(reply, DBUS_TYPE_UINT32, &rid, DBUS_TYPE_INVALID);
				dbus_connection_send(c, reply, NULL);
				dbus_connection_flush(c);
				dbus_message_unref(reply);
			}
			else if (dbus_message_is_method_call(m, "org.freedesktop.Notifications", "CloseNotification"))
			{
				dbus_uint32_t close_id = 0;
				dbus_message_get_args(m, NULL, DBUS_TYPE_UINT32, &close_id, DBUS_TYPE_INVALID);
				char line[1024];
				snprintf(line, sizeof(line), "CloseNotification\t%u", close_id);
				record(line);

				DBusMessage *reply = dbus_message_new_method_return(m);
				dbus_connection_send(c, reply, NULL);
				dbus_connection_flush(c);
				dbus_message_unref(reply);

				/* Emit NotificationClosed signal with reason 3 (closed by call) */
				DBusMessage *sig = dbus_message_new_signal("/org/freedesktop/Notifications",
					"org.freedesktop.Notifications", "NotificationClosed");
				dbus_uint32_t reason = 3;
				dbus_message_append_args(sig, DBUS_TYPE_UINT32, &close_id, DBUS_TYPE_UINT32, &reason, DBUS_TYPE_INVALID);
				dbus_connection_send(c, sig, NULL);
				dbus_connection_flush(c);
				dbus_message_unref(sig);
			}
			else if (dbus_message_is_method_call(m, "org.freedesktop.Notifications", "TestEmitActionInvoked"))
			{
				dbus_uint32_t target_id = 0;
				const char *action_key = NULL;
				dbus_message_get_args(m, NULL, DBUS_TYPE_UINT32, &target_id, DBUS_TYPE_STRING, &action_key, DBUS_TYPE_INVALID);

				DBusMessage *sig = dbus_message_new_signal("/org/freedesktop/Notifications",
					"org.freedesktop.Notifications", "ActionInvoked");
				dbus_message_append_args(sig, DBUS_TYPE_UINT32, &target_id, DBUS_TYPE_STRING, &action_key, DBUS_TYPE_INVALID);
				dbus_connection_send(c, sig, NULL);
				dbus_connection_flush(c);
				dbus_message_unref(sig);

				DBusMessage *reply = dbus_message_new_method_return(m);
				dbus_connection_send(c, reply, NULL);
				dbus_connection_flush(c);
				dbus_message_unref(reply);
			}
			else if (dbus_message_is_method_call(m, "org.freedesktop.DBus.Peer", "Ping"))
			{
				DBusMessage *reply = dbus_message_new_method_return(m);
				dbus_connection_send(c, reply, NULL);
				dbus_connection_flush(c);
				dbus_message_unref(reply);
			}
			else if (dbus_message_is_method_call(m, "org.freedesktop.portal.Notification", "AddNotification"))
			{
				const char *nid = NULL;
				DBusMessageIter it, dict_it;
				const char *title = NULL, *body = NULL;
				char line[8192];
				dbus_message_iter_init(m, &it);
				if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_STRING)
					dbus_message_iter_get_basic(&it, &nid);
				dbus_message_iter_next(&it);
				if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_ARRAY)
				{
					dbus_message_iter_recurse(&it, &dict_it);
					while (dbus_message_iter_get_arg_type(&dict_it) == DBUS_TYPE_DICT_ENTRY)
					{
						DBusMessageIter entry, val;
						const char *key = NULL;
						dbus_message_iter_recurse(&dict_it, &entry);
						dbus_message_iter_get_basic(&entry, &key);
						dbus_message_iter_next(&entry);
						dbus_message_iter_recurse(&entry, &val);
						if (key && strcmp(key, "title") == 0)
							dbus_message_iter_get_basic(&val, &title);
						else if (key && strcmp(key, "body") == 0)
							dbus_message_iter_get_basic(&val, &body);
						dbus_message_iter_next(&dict_it);
					}
				}
				snprintf(line, sizeof(line), "PortalAddNotification\t%s\t%s\t%s",
					nid ? nid : "", title ? title : "", body ? body : "");
				record(line);

				DBusMessage *reply = dbus_message_new_method_return(m);
				dbus_connection_send(c, reply, NULL);
				dbus_connection_flush(c);
				dbus_message_unref(reply);
			}
			else if (dbus_message_is_method_call(m, "org.freedesktop.portal.Notification", "RemoveNotification"))
			{
				const char *nid = NULL;
				dbus_message_get_args(m, NULL, DBUS_TYPE_STRING, &nid, DBUS_TYPE_INVALID);
				char line[1024];
				snprintf(line, sizeof(line), "PortalRemoveNotification\t%s", nid ? nid : "");
				record(line);

				DBusMessage *reply = dbus_message_new_method_return(m);
				dbus_connection_send(c, reply, NULL);
				dbus_connection_flush(c);
				dbus_message_unref(reply);
			}
			else if (dbus_message_is_method_call(m, "org.freedesktop.portal.Notification", "TestEmitPortalActionInvoked"))
			{
				const char *nid = NULL;
				const char *action_key = NULL;
				dbus_message_get_args(m, NULL, DBUS_TYPE_STRING, &nid, DBUS_TYPE_STRING, &action_key, DBUS_TYPE_INVALID);

				DBusMessage *sig = dbus_message_new_signal("/org/freedesktop/portal/desktop",
					"org.freedesktop.portal.Notification", "ActionInvoked");
				dbus_message_append_args(sig, DBUS_TYPE_STRING, &nid, DBUS_TYPE_STRING, &action_key, DBUS_TYPE_INVALID);
				dbus_connection_send(c, sig, NULL);
				dbus_connection_flush(c);
				dbus_message_unref(sig);

				DBusMessage *reply = dbus_message_new_method_return(m);
				dbus_connection_send(c, reply, NULL);
				dbus_connection_flush(c);
				dbus_message_unref(reply);
			}
			else if (dbus_message_is_method_call(m, "org.freedesktop.Notifications", "TestEmitClosed"))
			{
				dbus_uint32_t target_id = 0;
				dbus_uint32_t reason = 2; /* dismissed by user */
				dbus_message_get_args(m, NULL, DBUS_TYPE_UINT32, &target_id, DBUS_TYPE_UINT32, &reason, DBUS_TYPE_INVALID);

				DBusMessage *sig = dbus_message_new_signal("/org/freedesktop/Notifications",
					"org.freedesktop.Notifications", "NotificationClosed");
				dbus_message_append_args(sig, DBUS_TYPE_UINT32, &target_id, DBUS_TYPE_UINT32, &reason, DBUS_TYPE_INVALID);
				dbus_connection_send(c, sig, NULL);
				dbus_connection_flush(c);
				dbus_message_unref(sig);

				DBusMessage *reply = dbus_message_new_method_return(m);
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
	char ready;
	if (pipe(fds) != 0)
		return -1;
	pid_t pid = fork();
	if (pid == 0)
	{
		close(fds[0]);
		serve_server(fds[1]);
	}
	close(fds[1]);
	if (read(fds[0], &ready, 1) != 1)
	{
		close(fds[0]);
		waitpid(pid, NULL, 0);
		return -1;
	}
	close(fds[0]);
	return pid;
}

static void stop_server(pid_t pid)
{
	kill(pid, SIGTERM);
	waitpid(pid, NULL, 0);
}

static int last_record(char *out, size_t size)
{
	FILE *f = fopen(record_path, "r");
	char line[8192];
	int n = 0;
	if (!f) return 0;
	while (fgets(line, sizeof(line), f))
	{
		n++;
		line[strcspn(line, "\n")] = 0;
		snprintf(out, size, "%s", line);
	}
	fclose(f);
	return n;
}

static uint32_t g_last_action_id = 0;
static char g_last_action_key[256] = "";
static void test_action_callback(uint32_t id, const char* action_key, void* context)
{
	g_last_action_id = id;
	snprintf(g_last_action_key, sizeof(g_last_action_key), "%s", action_key ? action_key : "");
}

static uint32_t g_last_closed_id = 0;
static uint32_t g_last_closed_reason = 0;
static void test_closed_callback(uint32_t id, uint32_t reason, void* context)
{
	g_last_closed_id = id;
	g_last_closed_reason = reason;
}

static void emit_test_action(DBusConnection *conn, uint32_t id, const char *action_key)
{
	DBusMessage *msg = dbus_message_new_method_call("org.freedesktop.Notifications",
		"/org/freedesktop/Notifications", "org.freedesktop.Notifications", "TestEmitActionInvoked");
	dbus_uint32_t mid = id;
	dbus_message_append_args(msg, DBUS_TYPE_UINT32, &mid, DBUS_TYPE_STRING, &action_key, DBUS_TYPE_INVALID);
	DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, 5000, NULL);
	if (reply) dbus_message_unref(reply);
	dbus_message_unref(msg);
}

static void emit_test_closed(DBusConnection *conn, uint32_t id, uint32_t reason)
{
	DBusMessage *msg = dbus_message_new_method_call("org.freedesktop.Notifications",
		"/org/freedesktop/Notifications", "org.freedesktop.Notifications", "TestEmitClosed");
	dbus_uint32_t mid = id;
	dbus_uint32_t mreason = reason;
	dbus_message_append_args(msg, DBUS_TYPE_UINT32, &mid, DBUS_TYPE_UINT32, &mreason, DBUS_TYPE_INVALID);
	DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, 5000, NULL);
	if (reply) dbus_message_unref(reply);
	dbus_message_unref(msg);
}

static void emit_test_portal_action(DBusConnection *conn, const char *id, const char *action_key)
{
	DBusMessage *msg = dbus_message_new_method_call("org.freedesktop.portal.Desktop",
		"/org/freedesktop/portal/desktop", "org.freedesktop.portal.Notification", "TestEmitPortalActionInvoked");
	dbus_message_append_args(msg, DBUS_TYPE_STRING, &id, DBUS_TYPE_STRING, &action_key, DBUS_TYPE_INVALID);
	DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, 5000, NULL);
	if (reply) dbus_message_unref(reply);
	dbus_message_unref(msg);
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "usage: probe RUN_DIR BUS_ADDRESS\n");
		return 2;
	}
	bus_dir = argv[1];
	bus_address = argv[2];
	snprintf(record_path, sizeof(record_path), "%s/records.log", bus_dir);

	env_address = bus_address;
	env_runtime_dir = NULL;

	pid_t server = start_server();
	if (server < 0) {
		fprintf(stderr, "FAIL: failed to start fake notification server\n");
		return 3;
	}

	DBusConnection *conn = _UNHostDBusConnect(&api);
	CHECK(conn != NULL);

	char rec[8192];

	/* Test 1: Basic notification with default action */
	uint32_t id1 = _UNHostDBusNotify(&api, conn, "TestApp", 0, "app-icon",
		"Test Summary", "Test Body", NULL, 0, NULL, NULL, 1, -1);
	CHECK(id1 == 1);
	CHECK(last_record(rec, sizeof(rec)) > 0);
	CHECK(strstr(rec, "Notify\t1\tTestApp\tTest Summary\tTest Body\t2\t1\t\t") != NULL);

	/* Test 2: Notification with custom actions */
	const char *actions[] = { "reply", "Reply", "delete", "Delete" };
	uint32_t id2 = _UNHostDBusNotify(&api, conn, "TestApp", 0, "app-icon",
		"Header — Subtitle", "Body text", actions, 4, NULL, NULL, 1, -1);
	CHECK(id2 == 2);
	CHECK(last_record(rec, sizeof(rec)) > 0);
	/* Total actions: 2 (default/open) + 4 (custom) = 6 */
	CHECK(strstr(rec, "Notify\t2\tTestApp\tHeader — Subtitle\tBody text\t6\t1\t\t") != NULL);

	/* Test 3: Notification with hints (urgency, image-path, sound-file) */
	uint32_t id3 = _UNHostDBusNotify(&api, conn, "TestApp", 0, "app-icon",
		"Critical Alert", "Urgent Body", NULL, 0, "/tmp/image.png", "default", 2, -1);
	CHECK(id3 == 3);
	CHECK(last_record(rec, sizeof(rec)) > 0);
	CHECK(strstr(rec, "Notify\t3\tTestApp\tCritical Alert\tUrgent Body\t2\t2\t/tmp/image.png\tdefault") != NULL);

	/* Test 4: Passive urgency hint (0) */
	uint32_t id4 = _UNHostDBusNotify(&api, conn, "TestApp", 0, "app-icon",
		"Passive Alert", "Quiet Body", NULL, 0, NULL, NULL, 0, -1);
	CHECK(id4 == 4);
	CHECK(last_record(rec, sizeof(rec)) > 0);
	CHECK(strstr(rec, "Notify\t4\tTestApp\tPassive Alert\tQuiet Body\t2\t0\t\t") != NULL);

	/* Test 5: CloseNotification */
	int close_res = _UNHostDBusCloseNotification(&api, conn, id1);
	CHECK(close_res == 0);
	CHECK(last_record(rec, sizeof(rec)) > 0);
	CHECK(strstr(rec, "CloseNotification\t1") != NULL);

	/* Test 6: ActionInvoked signal for plain click ("default") */
	emit_test_action(conn, id2, "default");
	usleep(50000);
	_UNHostDBusProcessMessages(&api, conn, test_action_callback, test_closed_callback, NULL);
	CHECK(g_last_action_id == id2);
	CHECK(strcmp(g_last_action_key, "default") == 0);

	/* Test 7: ActionInvoked signal for custom action ("reply") */
	emit_test_action(conn, id2, "reply");
	usleep(50000);
	_UNHostDBusProcessMessages(&api, conn, test_action_callback, test_closed_callback, NULL);
	CHECK(g_last_action_id == id2);
	CHECK(strcmp(g_last_action_key, "reply") == 0);

	/* Test 8: NotificationClosed signal */
	emit_test_closed(conn, id3, 2);
	usleep(50000);
	_UNHostDBusProcessMessages(&api, conn, test_action_callback, test_closed_callback, NULL);
	CHECK(g_last_closed_id == id3);
	CHECK(g_last_closed_reason == 2);

	/* Test 9: Portal availability */
	int portal_avail = _UNHostDBusIsPortalAvailable(&api, conn);
	CHECK(portal_avail == 1);

	/* Test 10: Portal AddNotification */
	const char *portal_actions[] = { "view", "View" };
	int p_add = _UNHostDBusPortalAddNotification(&api, conn, "99", "Portal Title", "Portal Body", "app-icon", portal_actions, 2);
	CHECK(p_add == 0);
	CHECK(last_record(rec, sizeof(rec)) > 0);
	CHECK(strstr(rec, "PortalAddNotification\t99\tPortal Title\tPortal Body") != NULL);

	/* Test 11: Portal RemoveNotification */
	int p_rem = _UNHostDBusPortalRemoveNotification(&api, conn, "99");
	CHECK(p_rem == 0);
	CHECK(last_record(rec, sizeof(rec)) > 0);
	CHECK(strstr(rec, "PortalRemoveNotification\t99") != NULL);

	/* Test 12: Portal ActionInvoked signal */
	emit_test_portal_action(conn, "99", "view");
	usleep(50000);
	_UNHostDBusProcessMessages(&api, conn, test_action_callback, test_closed_callback, NULL);
	CHECK(g_last_action_id == 99);
	CHECK(strcmp(g_last_action_key, "view") == 0);

	_UNHostDBusDisconnect(&api, conn);
	stop_server(server);

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

with tempfile.TemporaryDirectory(prefix="un-dbus-") as tmp:
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
                            "-fno-sanitize-recover=undefined", f"-I{usernoted}", probe_c,
                            os.path.join(usernoted, "usernoted_dbus.c"),
                            *flags.stdout.split(), "-o", os.path.join(tmp, "probe")],
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
        env.pop("DBUS_SESSION_BUS_ADDRESS", None)
        env["ASAN_OPTIONS"] = "detect_leaks=0"
        run = subprocess.run([os.path.join(tmp, "probe"), bus_dir, address], capture_output=True, text=True,
                             env=env, timeout=60)
        sys.stdout.write(run.stdout + run.stderr)
        if run.returncode != 0:
            sys.exit("FAIL: desktop notification delivery test failed")
    finally:
        daemon.terminate()
        daemon.wait()

print("PASS: desktop notification delivery via org.freedesktop.Notifications verified")
