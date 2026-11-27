#!/usr/bin/env python3
# Host test: the ScreenCast portal client talks only to a private bus and a
# fake portal. It checks the documented session lifecycle, stream metadata and
# PipeWire fd handoff without using the user's session bus.
# usage: python3 src/libpipewire/tests/screencast_portal_host.py [REPO_ROOT]

import os
import shutil
import subprocess
import sys
import tempfile

root = sys.argv[1] if len(sys.argv) > 1 else os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', '..'))

for tool in ("cc", "pkg-config", "dbus-daemon"):
    if shutil.which(tool) is None:
        print(f"SKIP: {tool} is not installed")
        sys.exit(0)

PROBE = r"""
#include "screencast_portal.h"
#include "darling_dbus.h"
#include <dbus/dbus.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

enum { PORTAL_OK, PORTAL_DENY, PORTAL_CANCEL, PORTAL_NO_STREAMS };

#define SCREENCAST_IFACE "org.freedesktop.portal.ScreenCast"
#define REQUEST_IFACE "org.freedesktop.portal.Request"
#define SESSION_PATH "/org/freedesktop/portal/desktop/session/screencast_test"
#define REQUEST_PATH "/org/freedesktop/portal/desktop/request/screencast_test"

static const char *bus_address;
static char record_path[4096];
static int failures;
static const char *env_address;
static struct darling_dbus dbus_table;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static char *fake_getenv(const char *name)
{
	return strcmp(name, "DBUS_SESSION_BUS_ADDRESS") == 0 ? (char *) env_address : NULL;
}

static struct darling_screencast_portal_api api = { &dbus_table, fake_getenv, 1000 };

static void record(const char *format, ...)
{
	FILE *file = fopen(record_path, "a");
	va_list args;
	if (!file) return;
	va_start(args, format);
	vfprintf(file, format, args);
	va_end(args);
	fputs("\n", file);
	fclose(file);
}

static int find_variant(DBusMessageIter *args, const char *key, DBusMessageIter *value)
{
	DBusMessageIter options, entry, variant;
	const char *name = NULL;

	if (dbus_message_iter_get_arg_type(args) != DBUS_TYPE_ARRAY) return 0;
	dbus_message_iter_recurse(args, &options);
	while (dbus_message_iter_get_arg_type(&options) == DBUS_TYPE_DICT_ENTRY)
	{
		dbus_message_iter_recurse(&options, &entry);
		if (dbus_message_iter_get_arg_type(&entry) == DBUS_TYPE_STRING)
			dbus_message_iter_get_basic(&entry, &name);
		if (dbus_message_iter_next(&entry)
			&& dbus_message_iter_get_arg_type(&entry) == DBUS_TYPE_VARIANT)
		{
			dbus_message_iter_recurse(&entry, &variant);
			if (name && strcmp(name, key) == 0)
			{
				*value = variant;
				return 1;
			}
		}
		dbus_message_iter_next(&options);
	}
	return 0;
}

static void add_prop_uint32(DBusMessageIter *options, const char *key, dbus_uint32_t value)
{
	DBusMessageIter entry, variant;
	dbus_message_iter_open_container(options, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
	dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
	dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "u", &variant);
	dbus_message_iter_append_basic(&variant, DBUS_TYPE_UINT32, &value);
	dbus_message_iter_close_container(&entry, &variant);
	dbus_message_iter_close_container(options, &entry);
}

static void add_prop_uint64(DBusMessageIter *options, const char *key, dbus_uint64_t value)
{
	DBusMessageIter entry, variant;
	dbus_message_iter_open_container(options, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
	dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
	dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "t", &variant);
	dbus_message_iter_append_basic(&variant, DBUS_TYPE_UINT64, &value);
	dbus_message_iter_close_container(&entry, &variant);
	dbus_message_iter_close_container(options, &entry);
}

static void add_prop_string(DBusMessageIter *options, const char *key, const char *value)
{
	DBusMessageIter entry, variant;
	dbus_message_iter_open_container(options, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
	dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
	dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant);
	dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &value);
	dbus_message_iter_close_container(&entry, &variant);
	dbus_message_iter_close_container(options, &entry);
}

static void add_prop_pair(DBusMessageIter *options, const char *key, int32_t a, int32_t b)
{
	DBusMessageIter entry, variant, tuple;
	dbus_message_iter_open_container(options, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
	dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
	dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "(ii)", &variant);
	dbus_message_iter_open_container(&variant, DBUS_TYPE_STRUCT, NULL, &tuple);
	dbus_message_iter_append_basic(&tuple, DBUS_TYPE_INT32, &a);
	dbus_message_iter_append_basic(&tuple, DBUS_TYPE_INT32, &b);
	dbus_message_iter_close_container(&variant, &tuple);
	dbus_message_iter_close_container(&entry, &variant);
	dbus_message_iter_close_container(options, &entry);
}

static void build_create_response(DBusMessageIter *options)
{
	add_prop_string(options, "session_handle", SESSION_PATH);
}

static void build_start_response(DBusMessageIter *options)
{
	DBusMessageIter entry, variant, array, stream, props;
	const char *key = "streams";
	const char *signature = "(ua{sv})";
	dbus_uint32_t node_id = 42;
	dbus_uint32_t source_type = 1;
	dbus_uint64_t serial = 123456789ULL;

	dbus_message_iter_open_container(options, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
	dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
	dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "a(ua{sv})", &variant);
	dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY, signature, &array);
	dbus_message_iter_open_container(&array, DBUS_TYPE_STRUCT, NULL, &stream);
	dbus_message_iter_append_basic(&stream, DBUS_TYPE_UINT32, &node_id);
	dbus_message_iter_open_container(&stream, DBUS_TYPE_ARRAY, "{sv}", &props);
	add_prop_pair(&props, "size", 1920, 1080);
	add_prop_pair(&props, "position", 0, 0);
	add_prop_uint32(&props, "source_type", source_type);
	add_prop_uint64(&props, "pipewire-serial", serial);
	add_prop_string(&props, "id", "monitor-0");
	dbus_message_iter_close_container(&stream, &props);
	dbus_message_iter_close_container(&array, &stream);
	dbus_message_iter_close_container(&variant, &array);
	dbus_message_iter_close_container(&entry, &variant);
	dbus_message_iter_close_container(options, &entry);
}

static void send_response(DBusConnection *connection, dbus_uint32_t code,
	void (*build)(DBusMessageIter *))
{
	DBusMessage *signal = dbus_message_new_signal(REQUEST_PATH, REQUEST_IFACE, "Response");
	DBusMessageIter args, options;
	dbus_message_iter_init_append(signal, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &code);
	dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &options);
	if (build) build(&options);
	dbus_message_iter_close_container(&args, &options);
	dbus_connection_send(connection, signal, NULL);
	dbus_connection_flush(connection);
	dbus_message_unref(signal);
}

static void record_options(DBusMessage *message)
{
	DBusMessageIter args, value;
	dbus_uint32_t types = 0, cursor = 0;
	dbus_bool_t multiple = 0;
	const char *restore = "";

	if (!dbus_message_iter_init(message, &args)) return;
	if (dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_OBJECT_PATH)
		dbus_message_iter_next(&args);
	if (dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_STRING)
		dbus_message_iter_next(&args);
	if (find_variant(&args, "types", &value))
		dbus_message_iter_get_basic(&value, &types);
	if (find_variant(&args, "cursor_mode", &value))
		dbus_message_iter_get_basic(&value, &cursor);
	if (find_variant(&args, "multiple", &value))
		dbus_message_iter_get_basic(&value, &multiple);
	if (find_variant(&args, "restore_token", &value))
		dbus_message_iter_get_basic(&value, &restore);
	record("SelectSources\t%s\t%u\t%u\t%d\t%s", SESSION_PATH, types, cursor,
		multiple != 0, restore);
}

static void serve_portal(int mode, int ready_fd)
{
	DBusError error;
	DBusConnection *connection;
	const char *request = REQUEST_PATH;

	dbus_error_init(&error);
	connection = dbus_connection_open_private(bus_address, &error);
	if (!connection || !dbus_bus_register(connection, &error)
		|| dbus_bus_request_name(connection, "org.freedesktop.portal.Desktop",
			DBUS_NAME_FLAG_DO_NOT_QUEUE, &error) != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
		_exit(3);
	if (write(ready_fd, "r", 1) != 1) _exit(3);
	close(ready_fd);

	while (dbus_connection_read_write(connection, -1))
	{
		DBusMessage *message;
		while ((message = dbus_connection_pop_message(connection)) != NULL)
		{
			DBusMessage *reply = NULL;
			if (dbus_message_is_method_call(message, "org.freedesktop.DBus.Properties", "Get"))
			{
				DBusMessageIter args, variant;
				const char *interface = NULL, *property = NULL;
				dbus_uint32_t value = 0;

				dbus_message_iter_init(message, &args);
				dbus_message_iter_get_basic(&args, &interface);
				dbus_message_iter_next(&args);
				dbus_message_iter_get_basic(&args, &property);
				record("Get\t%s\t%s", interface ? interface : "", property ? property : "");
				if (property && strcmp(property, "AvailableSourceTypes") == 0) value = 7;
				else if (property && strcmp(property, "AvailableCursorModes") == 0) value = 7;
				else if (property && strcmp(property, "version") == 0) value = 6;
				reply = dbus_message_new_method_return(message);
				dbus_message_iter_init_append(reply, &args);
				dbus_message_iter_open_container(&args, DBUS_TYPE_VARIANT, "u", &variant);
				dbus_message_iter_append_basic(&variant, DBUS_TYPE_UINT32, &value);
				dbus_message_iter_close_container(&args, &variant);
			}
			else if (dbus_message_is_method_call(message, SCREENCAST_IFACE, "CreateSession"))
			{
				DBusMessageIter args;
				dbus_bool_t had_token = 0;
				if (dbus_message_iter_init(message, &args))
					had_token = find_variant(&args, "handle_token", &args);
				record("CreateSession\t%d", had_token);
				reply = dbus_message_new_method_return(message);
				dbus_message_append_args(reply, DBUS_TYPE_OBJECT_PATH, &request,
					DBUS_TYPE_INVALID);
			}
			else if (dbus_message_is_method_call(message, SCREENCAST_IFACE, "SelectSources"))
			{
				record_options(message);
				reply = dbus_message_new_method_return(message);
				dbus_message_append_args(reply, DBUS_TYPE_OBJECT_PATH, &request,
					DBUS_TYPE_INVALID);
			}
			else if (dbus_message_is_method_call(message, SCREENCAST_IFACE, "Start"))
			{
				record("Start\t%s\t%s", SESSION_PATH, dbus_message_get_signature(message));
				reply = dbus_message_new_method_return(message);
				dbus_message_append_args(reply, DBUS_TYPE_OBJECT_PATH, &request,
					DBUS_TYPE_INVALID);
			}
			else if (dbus_message_is_method_call(message, SCREENCAST_IFACE, "OpenPipeWireRemote"))
			{
				int pair[2];
				record("OpenPipeWireRemote\t%s", SESSION_PATH);
				reply = dbus_message_new_method_return(message);
				if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, pair) == 0)
				{
					dbus_message_append_args(reply, DBUS_TYPE_UNIX_FD, &pair[0],
						DBUS_TYPE_INVALID);
					close(pair[0]);
					close(pair[1]);
				}
			}
			else if (dbus_message_is_method_call(message, "org.freedesktop.portal.Session", "Close"))
			{
				record("Close\t%s", SESSION_PATH);
				reply = dbus_message_new_method_return(message);
			}

			if (reply)
			{
				dbus_connection_send(connection, reply, NULL);
				dbus_connection_flush(connection);
				if (dbus_message_is_method_call(message, SCREENCAST_IFACE, "CreateSession"))
				{
					if (mode == PORTAL_DENY) send_response(connection, 2, NULL);
					else if (mode == PORTAL_CANCEL) send_response(connection, 1, NULL);
					else send_response(connection, 0, build_create_response);
				}
				else if (dbus_message_is_method_call(message, SCREENCAST_IFACE, "SelectSources"))
					send_response(connection, 0, NULL);
				else if (dbus_message_is_method_call(message, SCREENCAST_IFACE, "Start"))
				{
					if (mode == PORTAL_NO_STREAMS) send_response(connection, 0, NULL);
					else send_response(connection, 0, build_start_response);
				}
				dbus_message_unref(reply);
			}
			dbus_message_unref(message);
		}
	}
	_exit(0);
}

static pid_t start_portal(int mode)
{
	int fds[2];
	char ready;
	pid_t pid;

	if (pipe(fds) != 0) return -1;
	pid = fork();
	if (pid == 0)
	{
		close(fds[0]);
		serve_portal(mode, fds[1]);
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

static void stop_portal(pid_t pid)
{
	kill(pid, SIGTERM);
	waitpid(pid, NULL, 0);
}

static int record_lines(char lines[][4096], int maximum)
{
	FILE *file = fopen(record_path, "r");
	int count = 0;
	if (!file) return 0;
	while (count < maximum && fgets(lines[count], 4096, file))
	{
		lines[count][strcspn(lines[count], "\n")] = '\0';
		count++;
	}
	fclose(file);
	return count;
}

static void reset(void)
{
	unlink(record_path);
	env_address = bus_address;
	api.timeout_ms = 1000;
}

int main(int argc, char **argv)
{
	char lines[12][4096];
	int count;
	pid_t portal;
	uint32_t sources = 0, cursors = 0, version = 0;
	struct darling_screencast_config config = {
		DARLING_SCREENCAST_SOURCE_MONITOR | DARLING_SCREENCAST_SOURCE_WINDOW,
		DARLING_SCREENCAST_CURSOR_EMBEDDED, 1, 0, "restore-me" };

	if (argc != 3) return 2;
	bus_address = argv[2];
	snprintf(record_path, sizeof(record_path), "%s/record", argv[1]);

	if (darling_dbus_load("libdbus-1.so.3", 0, &dbus_table) != 0)
	{
		puts("FAIL: host libdbus-1 does not expose the required functions");
		return 1;
	}

	reset();
	CHECK(darling_screencast_portal_properties(&api, &sources, &cursors, &version)
		== DARLING_SCREENCAST_NOT_AVAILABLE);
	CHECK(darling_screencast_session_create(&api, &config,
		&(struct darling_screencast_session *) { NULL }) == DARLING_SCREENCAST_NOT_AVAILABLE);

	reset();
	portal = start_portal(PORTAL_OK);
	CHECK(portal > 0);
	CHECK(darling_screencast_portal_properties(&api, &sources, &cursors, &version)
		== DARLING_SCREENCAST_OK);
	CHECK(sources == 7 && cursors == 7 && version == 6);
	{
		struct darling_screencast_session *session = NULL;
		const struct darling_screencast_stream *streams;
		uint32_t stream_count = 0;

		CHECK(darling_screencast_session_create(&api, &config, &session)
			== DARLING_SCREENCAST_OK);
		CHECK(session != NULL);
		if (session)
		{
			CHECK(darling_screencast_session_fd(session) >= 0);
			CHECK(fcntl(darling_screencast_session_fd(session), F_GETFD) >= 0);
			streams = darling_screencast_session_streams(session, &stream_count);
			CHECK(stream_count == 1);
			if (stream_count == 1)
			{
				CHECK(streams[0].node_id == 42);
				CHECK(streams[0].serial == 123456789ULL);
				CHECK(streams[0].source_type == DARLING_SCREENCAST_SOURCE_MONITOR);
				CHECK(streams[0].width == 1920 && streams[0].height == 1080);
				CHECK(streams[0].x == 0 && streams[0].y == 0);
				CHECK(strcmp(streams[0].id, "monitor-0") == 0);
				CHECK(strcmp(streams[0].target_object, "123456789") == 0);
			}
			darling_screencast_session_destroy(session);
		}
	}
	count = record_lines(lines, 12);
	CHECK(count == 8);
	if (count == 8)
	{
		CHECK(strcmp(lines[0], "Get\torg.freedesktop.portal.ScreenCast\tAvailableSourceTypes") == 0);
		CHECK(strcmp(lines[1], "Get\torg.freedesktop.portal.ScreenCast\tAvailableCursorModes") == 0);
		CHECK(strcmp(lines[2], "Get\torg.freedesktop.portal.ScreenCast\tversion") == 0);
		CHECK(strcmp(lines[3], "CreateSession\t1") == 0);
		CHECK(strcmp(lines[4], "SelectSources\t" SESSION_PATH "\t3\t2\t1\trestore-me") == 0);
		CHECK(strcmp(lines[5], "Start\t" SESSION_PATH "\tosa{sv}") == 0);
		CHECK(strcmp(lines[6], "OpenPipeWireRemote\t" SESSION_PATH) == 0);
		CHECK(strcmp(lines[7], "Close\t" SESSION_PATH) == 0);
	}
	stop_portal(portal);

	reset();
	portal = start_portal(PORTAL_DENY);
	CHECK(portal > 0);
	CHECK(darling_screencast_session_create(&api, &config,
		&(struct darling_screencast_session *) { NULL }) == DARLING_SCREENCAST_DENIED);
	stop_portal(portal);

	reset();
	portal = start_portal(PORTAL_CANCEL);
	CHECK(portal > 0);
	CHECK(darling_screencast_session_create(&api, &config,
		&(struct darling_screencast_session *) { NULL }) == DARLING_SCREENCAST_CANCELLED);
	stop_portal(portal);

	reset();
	portal = start_portal(PORTAL_NO_STREAMS);
	CHECK(portal > 0);
	CHECK(darling_screencast_session_create(&api, &config,
		&(struct darling_screencast_session *) { NULL }) == DARLING_SCREENCAST_NO_STREAMS);
	stop_portal(portal);

	if (failures == 0)
		puts("PASS: ScreenCast portal client");
	fflush(stdout);
	return failures == 0 ? 0 : 1;
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

with tempfile.TemporaryDirectory(prefix="screencast-portal-") as tmp:
    bus_dir = os.path.join(tmp, "run dir")
    os.mkdir(bus_dir, 0o700)
    socket_path = os.path.join(bus_dir, "bus")

    probe_c = os.path.join(tmp, "probe.c")
    with open(probe_c, "w") as f:
        f.write(PROBE)
    flags = subprocess.run(["pkg-config", "--cflags", "--libs", "dbus-1"],
                           capture_output=True, text=True)
    if flags.returncode != 0:
        print("SKIP: libdbus-1 development files are not installed")
        sys.exit(0)
    build = subprocess.run([
        "cc", "-std=gnu11", "-Wall", "-g",
        f"-I{os.path.join(root, 'src/libpipewire/include')}",
        f"-I{os.path.join(root, 'src/libpipewire')}",
        f"-I{os.path.join(root, 'src/libdarlingdbus/include')}",
        probe_c,
        os.path.join(root, "src/libpipewire/portal_common.c"),
        os.path.join(root, "src/libpipewire/screencast_portal.c"),
        os.path.join(root, "src/libdarlingdbus/darling_dbus.c"),
        *flags.stdout.split(), "-ldl", "-lpthread", "-o", os.path.join(tmp, "probe"),
    ], capture_output=True, text=True)
    if build.returncode != 0:
        print(build.stdout + build.stderr)
        sys.exit("FAIL: the ScreenCast portal probe does not build")

    config = os.path.join(tmp, "bus.conf")
    with open(config, "w") as f:
        escaped = socket_path.replace("&", "&amp;").replace("<", "&lt;")
        f.write(CONFIG.format(address="unix:path=" + escaped.replace(" ", "%20")))
    daemon = subprocess.Popen(["dbus-daemon", f"--config-file={config}", "--nofork", "--nopidfile",
                               "--print-address"], stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                              text=True)
    try:
        address = daemon.stdout.readline().strip()
        if not address:
            print(daemon.stderr.read())
            sys.exit("FAIL: the private dbus-daemon did not start")
        address = address.split(",guid=")[0]
        env = dict(os.environ)
        env.pop("DBUS_SESSION_BUS_ADDRESS", None)
        run = subprocess.run([os.path.join(tmp, "probe"), bus_dir, address],
                             capture_output=True, text=True, env=env, timeout=60)
        sys.stdout.write(run.stdout + run.stderr)
        if run.returncode != 0:
            sys.exit("FAIL: ScreenCast portal client regression")
    finally:
        daemon.terminate()
        daemon.wait()
