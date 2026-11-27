#!/usr/bin/env python3
# Host test: the org.freedesktop.portal.Camera client speaks only to a private
# bus and a fake portal. It checks the documented property, request/response
# and PipeWire fd handoff without involving the user's session bus.
# usage: python3 src/libpipewire/tests/camera_portal_host.py [REPO_ROOT]

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
#include "camera_portal.h"
#include "darling_dbus.h"
#include <dbus/dbus.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

enum { PORTAL_PRESENT, PORTAL_ABSENT, PORTAL_DENY, PORTAL_CANCEL, PORTAL_ERROR };

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

static struct darling_camera_portal_api api = { &dbus_table, fake_getenv, 1000 };

static void record(const char *line)
{
	FILE *f = fopen(record_path, "a");
	if (f) { fprintf(f, "%s\n", line); fclose(f); }
}

static const char *token_from_options(DBusMessage *message)
{
	DBusMessageIter args, options, entry, value;
	static char token[256];
	const char *key = NULL, *text = NULL;

	token[0] = '\0';
	if (!dbus_message_iter_init(message, &args)) return token;
	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY) return token;
	dbus_message_iter_recurse(&args, &options);
	while (dbus_message_iter_get_arg_type(&options) == DBUS_TYPE_DICT_ENTRY)
	{
		dbus_message_iter_recurse(&options, &entry);
		if (dbus_message_iter_get_arg_type(&entry) == DBUS_TYPE_STRING)
			dbus_message_iter_get_basic(&entry, &key);
		if (dbus_message_iter_next(&entry) && dbus_message_iter_get_arg_type(&entry) == DBUS_TYPE_VARIANT)
		{
			dbus_message_iter_recurse(&entry, &value);
			if (dbus_message_iter_get_arg_type(&value) == DBUS_TYPE_STRING)
				dbus_message_iter_get_basic(&value, &text);
		}
		if (key && text && strcmp(key, "handle_token") == 0)
		{
			snprintf(token, sizeof(token), "%s", text);
			return token;
		}
		dbus_message_iter_next(&options);
	}
	return token;
}

static void send_response(DBusConnection *connection, const char *handle, dbus_uint32_t code)
{
	DBusMessage *signal = dbus_message_new_signal(handle,
		"org.freedesktop.portal.Request", "Response");
	dbus_message_append_args(signal, DBUS_TYPE_UINT32, &code, DBUS_TYPE_INVALID);
	dbus_connection_send(connection, signal, NULL);
	dbus_connection_flush(connection);
	dbus_message_unref(signal);
}

static void serve_portal(int mode, int ready_fd)
{
	DBusError error;
	DBusConnection *connection;
	const char *handle = "/org/freedesktop/portal/desktop/request/camera_test";
	char line[4096];

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
				dbus_bool_t present = mode == PORTAL_PRESENT || mode == PORTAL_DENY;

				dbus_message_iter_init(message, &args);
				dbus_message_iter_get_basic(&args, &interface);
				dbus_message_iter_next(&args);
				dbus_message_iter_get_basic(&args, &property);
				snprintf(line, sizeof(line), "Get\t%s\t%s", interface ? interface : "", property ? property : "");
				record(line);
				if (mode == PORTAL_ERROR)
					reply = dbus_message_new_error(message,
						"org.freedesktop.DBus.Error.Failed", "property query failed");
				else if (interface && property && strcmp(interface, "org.freedesktop.portal.Camera") == 0
					&& strcmp(property, "IsCameraPresent") == 0)
				{
					reply = dbus_message_new_method_return(message);
					dbus_message_iter_init_append(reply, &args);
					dbus_message_iter_open_container(&args, DBUS_TYPE_VARIANT, "b", &variant);
					dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &present);
					dbus_message_iter_close_container(&args, &variant);
				}
				else
					reply = dbus_message_new_error(message,
						"org.freedesktop.DBus.Error.UnknownProperty", "unknown");
			}
			else if (dbus_message_is_method_call(message, "org.freedesktop.portal.Camera", "AccessCamera"))
			{
				const char *token = token_from_options(message);
				snprintf(line, sizeof(line), "AccessCamera\t%s\t%s", token, dbus_message_get_signature(message));
				record(line);
				reply = dbus_message_new_method_return(message);
				dbus_message_append_args(reply, DBUS_TYPE_OBJECT_PATH, &handle, DBUS_TYPE_INVALID);
			}
			else if (dbus_message_is_method_call(message, "org.freedesktop.portal.Camera", "OpenPipeWireRemote"))
			{
				int pair[2];
				snprintf(line, sizeof(line), "OpenPipeWireRemote\t%s", dbus_message_get_signature(message));
				record(line);
				reply = dbus_message_new_method_return(message);
				if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, pair) == 0)
				{
					dbus_message_append_args(reply, DBUS_TYPE_UNIX_FD, &pair[0], DBUS_TYPE_INVALID);
					close(pair[0]);
					close(pair[1]);
				}
			}

			if (reply)
			{
				dbus_connection_send(connection, reply, NULL);
				dbus_connection_flush(connection);
				if (dbus_message_is_method_call(message, "org.freedesktop.portal.Camera", "AccessCamera"))
				{
					dbus_uint32_t code = 0;
					if (mode == PORTAL_CANCEL)
						code = 1;
					else if (mode == PORTAL_DENY)
						code = 2;
					send_response(connection, handle, code);
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
	FILE *f = fopen(record_path, "r");
	int count = 0;
	if (!f) return 0;
	while (count < maximum && fgets(lines[count], 4096, f))
	{
		lines[count][strcspn(lines[count], "\n")] = '\0';
		count++;
	}
	fclose(f);
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
	char lines[8][4096];
	int count;
	pid_t portal;

	if (argc != 3) return 2;
	bus_address = argv[2];
	snprintf(record_path, sizeof(record_path), "%s/record", argv[1]);

	if (darling_dbus_load("libdbus-1.so.3", 0, &dbus_table) != 0)
	{
		puts("FAIL: host libdbus-1 does not expose the required functions");
		return 1;
	}

	/* Nothing on the bus: every call reports the portal as unavailable. */
	reset();
	CHECK(darling_camera_portal_is_present(&api) == DARLING_CAMERA_PORTAL_NOT_AVAILABLE);
	CHECK(darling_camera_portal_request_access(&api) == DARLING_CAMERA_PORTAL_NOT_AVAILABLE);
	CHECK(darling_camera_portal_open_remote(&api, &(int) { -1 }) == DARLING_CAMERA_PORTAL_NOT_AVAILABLE);

	/* The documented happy path: property, handle token, response and fd. */
	reset();
	portal = start_portal(PORTAL_PRESENT);
	CHECK(portal > 0);
	CHECK(darling_camera_portal_is_present(&api) == DARLING_CAMERA_PORTAL_OK);
	CHECK(darling_camera_portal_request_access(&api) == DARLING_CAMERA_PORTAL_OK);
	{
		int fd = -1;
		CHECK(darling_camera_portal_open_remote(&api, &fd) == DARLING_CAMERA_PORTAL_OK);
		CHECK(fd >= 0 && fcntl(fd, F_GETFD) >= 0);
		if (fd >= 0) close(fd);
	}
	count = record_lines(lines, 8);
	CHECK(count == 3);
	if (count == 3)
	{
		CHECK(strcmp(lines[0], "Get\torg.freedesktop.portal.Camera\tIsCameraPresent") == 0);
		CHECK(strncmp(lines[1], "AccessCamera\t", 13) == 0);
		CHECK(strstr(lines[1], "a{sv}") != NULL && strlen(lines[1]) > 14);
		CHECK(strcmp(lines[2], "OpenPipeWireRemote\ta{sv}") == 0);
	}
	stop_portal(portal);

	/* A cancelled Request.Response is reported as cancellation. */
	reset();
	portal = start_portal(PORTAL_CANCEL);
	CHECK(portal > 0);
	CHECK(darling_camera_portal_request_access(&api) == DARLING_CAMERA_PORTAL_CANCELLED);
	stop_portal(portal);

	/* Any other Request.Response is reported as denial. */
	reset();
	portal = start_portal(PORTAL_DENY);
	CHECK(portal > 0);
	CHECK(darling_camera_portal_request_access(&api) == DARLING_CAMERA_PORTAL_DENIED);
	stop_portal(portal);

	/* A present but unsupported property is not mistaken for a camera. */
	reset();
	portal = start_portal(PORTAL_ABSENT);
	CHECK(portal > 0);
	CHECK(darling_camera_portal_is_present(&api) == DARLING_CAMERA_PORTAL_NOT_AVAILABLE);
	stop_portal(portal);

	reset();
	portal = start_portal(PORTAL_ERROR);
	CHECK(portal > 0);
	CHECK(darling_camera_portal_is_present(&api) == DARLING_CAMERA_PORTAL_FAILED);
	stop_portal(portal);

	if (failures == 0)
		puts("PASS: Camera portal client");
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

with tempfile.TemporaryDirectory(prefix="camera-portal-") as tmp:
    bus_dir = os.path.join(tmp, "run dir")
    os.mkdir(bus_dir, 0o700)
    socket_path = os.path.join(bus_dir, "bus")

    probe_c = os.path.join(tmp, "probe.c")
    with open(probe_c, "w") as f:
        f.write(PROBE)
    flags = subprocess.run(["pkg-config", "--cflags", "--libs", "dbus-1"], capture_output=True, text=True)
    if flags.returncode != 0:
        print("SKIP: libdbus-1 development files are not installed")
        sys.exit(0)
    build = subprocess.run([
        "cc", "-std=gnu11", "-Wall", "-g",
        f"-I{os.path.join(root, 'src/libpipewire/include')}",
        f"-I{os.path.join(root, 'src/libdarlingdbus/include')}",
        probe_c,
        os.path.join(root, "src/libpipewire/camera_portal.c"),
        os.path.join(root, "src/libdarlingdbus/darling_dbus.c"),
        *flags.stdout.split(), "-ldl", "-lpthread", "-o", os.path.join(tmp, "probe"),
    ], capture_output=True, text=True)
    if build.returncode != 0:
        print(build.stdout + build.stderr)
        sys.exit("FAIL: the Camera portal probe does not build")

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
        run = subprocess.run([os.path.join(tmp, "probe"), bus_dir, address], capture_output=True, text=True,
                             env=env, timeout=60)
        sys.stdout.write(run.stdout + run.stderr)
        if run.returncode != 0:
            sys.exit("FAIL: Camera portal client regression")
    finally:
        daemon.terminate()
        daemon.wait()
