#!/usr/bin/env python3
# Host test: LaunchServices hands web and mail URLs to the XDG desktop portal
# (org.freedesktop.portal.OpenURI) and to nothing else.
# usage: python3 tests/launchservices-host-open-portal.py [REPO_ROOT]
#
# LSHostOpenBus.c and LSHostOpenURL.c are built natively against the host's
# libdbus-1. A private dbus-daemon with no service directories (nothing can be
# activated, so no real portal or browser is reachable) and a fake portal
# forked from the probe stand in for the desktop. The user's session bus is
# never used.

import os
import shutil
import subprocess
import sys
import tempfile
import time

root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ls = os.path.join(root, "src/frameworks/CoreServices/src/LaunchServices")

for tool in ("cc", "pkg-config", "dbus-daemon"):
    if shutil.which(tool) is None:
        print(f"SKIP: {tool} is not installed")
        sys.exit(0)

PROBE = r"""
#include "LSHostOpenBus.h"
#include <dbus/dbus.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

enum { PORTAL_ACCEPT, PORTAL_DENY, PORTAL_HANG };

static const char *bus_dir, *bus_address;
static char record_path[4096], marker_path[4096];
static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); failures++; } } while (0)

/* The environment the code under test sees. */
static const char *env_address, *env_runtime_dir;
static char *fake_getenv(const char *name)
{
	if (strcmp(name, "DBUS_SESSION_BUS_ADDRESS") == 0)
		return (char *) env_address;
	if (strcmp(name, "XDG_RUNTIME_DIR") == 0)
		return (char *) env_runtime_dir;
	return NULL;
}

static struct ls_host_dbus api = {
	dbus_connection_open_private, dbus_bus_register, dbus_connection_close, dbus_connection_unref,
	dbus_message_new_method_call, dbus_message_iter_init_append, dbus_message_iter_append_basic,
	dbus_message_iter_open_container, dbus_message_iter_close_container,
	dbus_connection_send_with_reply_and_block, dbus_message_unref, dbus_error_init, dbus_error_free,
	dbus_address_escape_value, dbus_free, fake_getenv, 0,
};

/* ---- the fake portal (a child process) ---- */

static void record(const char *line)
{
	FILE *f = fopen(record_path, "a");
	if (f) { fprintf(f, "%s\n", line); fclose(f); }
}

static void serve_portal(int mode, int ready_fd)
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
			if (dbus_message_is_method_call(m, "org.freedesktop.portal.OpenURI", "OpenURI")
				&& strcmp(dbus_message_get_path(m), "/org/freedesktop/portal/desktop") == 0)
			{
				DBusMessageIter it, options;
				const char *parent = NULL, *uri = NULL;
				int count = 0;
				char line[8192];
				dbus_message_iter_init(m, &it);
				if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_STRING)
					dbus_message_iter_get_basic(&it, &parent);
				dbus_message_iter_next(&it);
				if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_STRING)
					dbus_message_iter_get_basic(&it, &uri);
				dbus_message_iter_next(&it);
				if (dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_ARRAY)
				{
					dbus_message_iter_recurse(&it, &options);
					while (dbus_message_iter_get_arg_type(&options) != DBUS_TYPE_INVALID)
					{
						count++;
						dbus_message_iter_next(&options);
					}
				}
				snprintf(line, sizeof(line), "OpenURI\t%s\t%s\t%d\t%s", parent ? parent : "(none)",
					uri ? uri : "(none)", count, dbus_message_get_signature(m));
				record(line);

				DBusMessage *reply = NULL;
				if (mode == PORTAL_ACCEPT)
				{
					const char *handle = "/org/freedesktop/portal/desktop/request/1_1/t";
					reply = dbus_message_new_method_return(m);
					dbus_message_append_args(reply, DBUS_TYPE_OBJECT_PATH, &handle, DBUS_TYPE_INVALID);
				}
				else if (mode == PORTAL_DENY)
					reply = dbus_message_new_error(m, "org.freedesktop.portal.Error.NotAllowed", "denied");
				if (reply)
				{
					dbus_connection_send(c, reply, NULL);
					dbus_connection_flush(c);
					dbus_message_unref(reply);
				}
			}
			dbus_message_unref(m);
		}
	}
	_exit(0);
}

static pid_t start_portal(int mode)
{
	int fds[2];
	char ready;
	if (pipe(fds) != 0)
		return -1;
	pid_t pid = fork();
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

static int calls_recorded(char *last, size_t size)
{
	FILE *f = fopen(record_path, "r");
	char line[8192];
	int n = 0;
	if (!f)
		return 0;
	while (fgets(line, sizeof(line), f))
	{
		n++;
		line[strcspn(line, "\n")] = 0;
		snprintf(last, size, "%s", line);
	}
	fclose(f);
	return n;
}

static void reset(void)
{
	unlink(record_path);
	env_address = bus_address;
	env_runtime_dir = NULL;
	api.timeout_ms = 0;
}

int main(int argc, char **argv)
{
	char last[8192];
	char expected[8192];
	pid_t portal;

	if (argc != 3)
		return 2;
	bus_dir = argv[1];
	bus_address = argv[2];
	snprintf(record_path, sizeof(record_path), "%s/record", argv[1]);
	snprintf(marker_path, sizeof(marker_path), "%s/marker", argv[1]);

	/* A portal that accepts: one OpenURI call with the URL, an empty parent window and no options. */
	reset();
	portal = start_portal(PORTAL_ACCEPT);
	CHECK(portal > 0);
	CHECK(_LSHostOpenURL(&api, "https://example.org/a?b=c#d") == LS_HOST_OPEN_OK);
	CHECK(calls_recorded(last, sizeof(last)) == 1);
	CHECK(strcmp(last, "OpenURI\t\thttps://example.org/a?b=c#d\t0\tssa{sv}") == 0);
	CHECK(_LSHostOpenURL(&api, "mailto:someone@example.org?subject=Hi%20there") == LS_HOST_OPEN_OK);
	CHECK(calls_recorded(last, sizeof(last)) == 2);

	/* A URL the policy refuses never reaches the bus. */
	reset();
	CHECK(_LSHostOpenURL(&api, "file:///etc/passwd") == LS_HOST_OPEN_INVALID_URL);
	CHECK(_LSHostOpenURL(&api, "https://e.org/a b --new-window") == LS_HOST_OPEN_INVALID_URL);
	CHECK(_LSHostOpenURL(&api, "mailto:x@e.org?attach=/etc/passwd") == LS_HOST_OPEN_INVALID_URL);
	CHECK(calls_recorded(last, sizeof(last)) == 0);

	/* No DBUS_SESSION_BUS_ADDRESS: $XDG_RUNTIME_DIR/bus, escaped (the directory name has a space). */
	reset();
	env_address = NULL;
	env_runtime_dir = bus_dir;
	CHECK(_LSHostOpenURL(&api, "http://example.org/") == LS_HOST_OPEN_OK);
	CHECK(calls_recorded(last, sizeof(last)) == 1);
	env_address = "";
	CHECK(_LSHostOpenURL(&api, "http://example.org/") == LS_HOST_OPEN_OK);
	{
		char* address = _LSHostOpenSessionAddress(&api);
		snprintf(expected, sizeof(expected), "unix:path=%s/bus", bus_dir);
		CHECK(address != NULL && strncmp(address, "unix:path=", 10) == 0 && strstr(address, " ") == NULL);
		free(address);
	}
	env_runtime_dir = "relative/dir";
	CHECK(_LSHostOpenURL(&api, "http://example.org/") == LS_HOST_OPEN_NO_PORTAL);
	env_runtime_dir = NULL;
	CHECK(_LSHostOpenURL(&api, "http://example.org/") == LS_HOST_OPEN_NO_PORTAL);
	stop_portal(portal);

	/* Addresses that are not one plain unix socket are refused before anything is started or opened. */
	reset();
	{
		static char unixexec[8192], list[8192], spaced[8192];
		snprintf(unixexec, sizeof(unixexec), "unixexec:path=/usr/bin/touch,argv1=%s", marker_path);
		snprintf(list, sizeof(list), "unix:path=/nonexistent;unixexec:path=/usr/bin/touch,argv1=%s", marker_path);
		snprintf(spaced, sizeof(spaced), "%s unixexec:path=/usr/bin/touch", bus_address);
		const char *bad[] = { unixexec, list, spaced, "autolaunch:", "tcp:host=127.0.0.1,port=1",
			"unix:path=/x\nunixexec:path=/bin/sh", "unix:path=/x,unixexec:path=/bin/sh", "unix:path=/x\xc3\xa9" };
		for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++)
		{
			env_address = bad[i];
			CHECK(_LSHostOpenSessionAddress(&api) == NULL);
			CHECK(_LSHostOpenURL(&api, "https://example.org/") == LS_HOST_OPEN_NO_PORTAL);
		}
		static char big[2048];
		memset(big, 'a', sizeof(big) - 1);
		memcpy(big, "unix:path=", 10);
		env_address = big;
		CHECK(_LSHostOpenURL(&api, "https://example.org/") == LS_HOST_OPEN_NO_PORTAL);
		struct stat st;
		CHECK(stat(marker_path, &st) != 0);
	}

	/* No portal on the bus: reported as such, nothing else is tried. */
	reset();
	CHECK(_LSHostOpenURL(&api, "https://example.org/") == LS_HOST_OPEN_NO_PORTAL);
	/* No bus at all. */
	snprintf(expected, sizeof(expected), "unix:path=%s/nonexistent", bus_dir);
	env_address = expected;
	CHECK(_LSHostOpenURL(&api, "https://example.org/") == LS_HOST_OPEN_NO_PORTAL);

	/* A portal that refuses is not bypassed. */
	reset();
	portal = start_portal(PORTAL_DENY);
	CHECK(portal > 0);
	CHECK(_LSHostOpenURL(&api, "https://example.org/") == LS_HOST_OPEN_FAILED);
	CHECK(calls_recorded(last, sizeof(last)) == 1);
	stop_portal(portal);

	/* A portal that does not answer: a failure after the timeout, with no second attempt. */
	reset();
	portal = start_portal(PORTAL_HANG);
	CHECK(portal > 0);
	api.timeout_ms = 300;
	CHECK(_LSHostOpenURL(&api, "https://example.org/") == LS_HOST_OPEN_FAILED);
	CHECK(calls_recorded(last, sizeof(last)) == 1);
	stop_portal(portal);

	if (failures == 0)
		puts("PASS: host URL opening through the desktop portal");
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

with tempfile.TemporaryDirectory(prefix="ls-portal-") as tmp:
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
    build = subprocess.run(["cc", "-std=gnu11", "-Wall", "-g", "-fsanitize=address,undefined",
                            "-fno-sanitize-recover=undefined", f"-I{ls}", probe_c,
                            os.path.join(ls, "LSHostOpenBus.c"), os.path.join(ls, "LSHostOpenURL.c"),
                            *flags.stdout.split(), "-o", os.path.join(tmp, "probe")],
                           capture_output=True, text=True)
    if build.returncode != 0:
        print(build.stdout + build.stderr)
        sys.exit("FAIL: the probe does not build")

    # dbus-daemon escapes the address itself; give it the path unescaped in XML.
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
            sys.exit("FAIL: host URL opening regression")
    finally:
        daemon.terminate()
        daemon.wait()
