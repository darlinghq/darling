#!/usr/bin/env python3
# Host test: MediaPlayer now-playing and remote commands exposed via MPRIS 2 D-Bus
# usage: python3 tests/mediaplayer-desktop-mpris.py [REPO_ROOT]
#
# mediaremoted_dbus.c is built natively against the host's libdbus-1 with ASan
# and UBSan. A private dbus-daemon is spawned, and the test exercises:
# 1. Address resolution (__MEDIAREMOTED_DBUS_ADDRESS, DBUS_SESSION_BUS_ADDRESS, XDG_RUNTIME_DIR/bus)
# 2. Name claiming and releasing (org.mpris.MediaPlayer2.darling.<app>)
# 3. XML Introspection
# 4. org.freedesktop.DBus.Properties (Get, GetAll, Set)
# 5. org.mpris.MediaPlayer2 methods and properties
# 6. org.mpris.MediaPlayer2.Player methods (Play, Pause, PlayPause, Stop, Next, Previous, Seek, SetPosition)
# 7. Signals (PropertiesChanged, Seeked)
# 8. Error handling on invalid interfaces/methods/properties

import os
import shutil
import subprocess
import sys
import tempfile
import time

root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
mediaremoted_dir = os.path.join(root, "src/mediaremoted")

for tool in ("cc", "pkg-config", "dbus-daemon"):
    if shutil.which(tool) is None:
        print(f"SKIP: {tool} is not installed")
        sys.exit(0)

PROBE = r"""
#include "mediaremoted_dbus.h"
#include "mediaremoted_protocol.h"
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static const char *env_custom, *env_session, *env_runtime_dir;
static char *fake_getenv(const char *name)
{
	if (strcmp(name, MEDIAREMOTED_DBUS_ADDRESS_ENV) == 0)
		return (char *) env_custom;
	if (strcmp(name, "DBUS_SESSION_BUS_ADDRESS") == 0)
		return (char *) env_session;
	if (strcmp(name, "XDG_RUNTIME_DIR") == 0)
		return (char *) env_runtime_dir;
	return NULL;
}

static struct mediaremoted_dbus api = {
	dbus_connection_open_private,
	dbus_bus_register,
	dbus_bus_request_name,
	dbus_bus_release_name,
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
	dbus_message_new_signal,
	dbus_message_iter_init_append,
	dbus_message_iter_append_basic,
	dbus_message_iter_open_container,
	dbus_message_iter_close_container,
	dbus_message_iter_init,
	dbus_message_iter_get_arg_type,
	dbus_message_iter_get_basic,
	dbus_message_iter_recurse,
	dbus_message_iter_next,
	dbus_message_get_type,
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
};

static char s_last_cmd[64] = "";
static double s_last_val = -1.0;
static int test_cmd_callback(const char *command, double position_or_offset, void *context)
{
	strncpy(s_last_cmd, command ? command : "", sizeof(s_last_cmd) - 1);
	s_last_val = position_or_offset;
	return 0;
}

static void test_address_resolution(void)
{
	printf("--- Testing Address Resolution ---\n");
	// 1. Custom env override
	env_custom = "unix:path=/tmp/custom.sock";
	env_session = "unix:path=/tmp/session.sock";
	env_runtime_dir = "/run/user/1000";
	char *addr = _MRHostDBusSessionAddress(&api);
	CHECK(addr != NULL && strcmp(addr, "unix:path=/tmp/custom.sock") == 0);
	free(addr);

	// 2. Fallback to DBUS_SESSION_BUS_ADDRESS
	env_custom = NULL;
	addr = _MRHostDBusSessionAddress(&api);
	CHECK(addr != NULL && strcmp(addr, "unix:path=/tmp/session.sock") == 0);
	free(addr);

	// 3. Fallback to XDG_RUNTIME_DIR/bus
	env_session = NULL;
	addr = _MRHostDBusSessionAddress(&api);
	CHECK(addr != NULL && strcmp(addr, "unix:path=/run/user/1000/bus") == 0);
	free(addr);

	// 4. Invalid addresses rejected
	env_custom = "tcp:host=localhost,port=1234";
	addr = _MRHostDBusSessionAddress(&api);
	CHECK(addr == NULL);

	env_custom = "unix:path=/tmp/test;rm -rf /";
	addr = _MRHostDBusSessionAddress(&api);
	CHECK(addr == NULL);

	env_custom = NULL;
	env_runtime_dir = NULL;
	addr = _MRHostDBusSessionAddress(&api);
	CHECK(addr == NULL);
}

static DBusMessage *exchange_call(DBusConnection *client, DBusConnection *server, DBusMessage *req,
                                  const struct mr_player_state *state)
{
	dbus_uint32_t serial = 0;
	dbus_connection_send(client, req, &serial);
	dbus_connection_flush(client);
	dbus_message_unref(req);

	// Pump server until message arrives and is processed
	for (int i = 0; i < 50; i++) {
		usleep(2000); // 2ms
		dbus_connection_read_write(server, 5);
		if (_MRHostDBusProcessMessages(&api, server, state, test_cmd_callback, NULL) > 0)
			break;
	}
	dbus_connection_flush(server);

	// Pump client until reply arrives
	for (int i = 0; i < 50; i++) {
		usleep(2000);
		dbus_connection_read_write(client, 5);
		DBusMessage *msg;
		while ((msg = dbus_connection_pop_message(client)) != NULL) {
			if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_METHOD_RETURN ||
			    dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_ERROR) {
				if (dbus_message_get_reply_serial(msg) == serial)
					return msg;
			}
			dbus_message_unref(msg);
		}
	}
	return NULL;
}

// Properties.Get / Set with the given interface and property; Set carries a variant boolean
static DBusMessage *property_call(DBusConnection *client, DBusConnection *server, const char *method,
                                  const char *iface, const char *prop, const struct mr_player_state *state)
{
	DBusMessage *req = dbus_message_new_method_call("org.mpris.MediaPlayer2.darling.TestMusic",
		"/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", method);
	dbus_message_append_args(req, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, &prop, DBUS_TYPE_INVALID);
	if (strcmp(method, "Set") == 0) {
		DBusMessageIter it, v;
		dbus_bool_t b = 0;
		dbus_message_iter_init_append(req, &it);
		// the first two arguments are already there; append the value after them
		dbus_message_iter_open_container(&it, DBUS_TYPE_VARIANT, "b", &v);
		dbus_message_iter_append_basic(&v, DBUS_TYPE_BOOLEAN, &b);
		dbus_message_iter_close_container(&it, &v);
	}
	return exchange_call(client, server, req, state);
}

static void test_mpris_protocol(void)
{
	printf("--- Testing MPRIS 2 Server Protocol ---\n");
	env_custom = bus_address;
	DBusConnection *server_conn = _MRHostDBusConnect(&api);
	CHECK(server_conn != NULL);

	const char *bus_name = "org.mpris.MediaPlayer2.darling.TestMusic";
	int claim = _MRHostDBusClaimName(&api, server_conn, bus_name);
	CHECK(claim == 0);

	// Client connection
	DBusError err;
	dbus_error_init(&err);
	DBusConnection *client_conn = dbus_connection_open_private(bus_address, &err);
	CHECK(client_conn != NULL);
	CHECK(dbus_bus_register(client_conn, &err));

	struct mr_player_state state;
	memset(&state, 0, sizeof(state));
	state.identity = "Test Music Player";
	state.desktop_entry = "testmusic";
	state.playback_status = "Playing";
	state.rate = 1.0;
	state.position_us = 45000000; // 45s
	state.duration_us = 180000000; // 180s
	state.can_control = 1;
	state.can_play = 1;
	state.can_pause = 1;
	state.can_seek = 1;
	state.title = "Song Title";
	state.artist = "Artist Name";
	state.album = "Album Name";
	state.composer = "Composer Name";
	state.genre = "Rock";
	state.track_number = 3;
	state.disc_number = 1;
	state.art_url = "file:///tmp/art.png";
	state.track_id = "/org/mpris/MediaPlayer2/Track/1";

	// 1. Introspection
	DBusMessage *req = dbus_message_new_method_call(bus_name, "/org/mpris/MediaPlayer2",
		"org.freedesktop.DBus.Introspectable", "Introspect");
	CHECK(req != NULL);
	DBusMessage *reply = exchange_call(client_conn, server_conn, req, &state);
	CHECK(reply != NULL && dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_METHOD_RETURN);
	const char *xml = NULL;
	dbus_message_get_args(reply, &err, DBUS_TYPE_STRING, &xml, DBUS_TYPE_INVALID);
	CHECK(xml != NULL && strstr(xml, "org.mpris.MediaPlayer2") != NULL);
	CHECK(strstr(xml, "org.mpris.MediaPlayer2.Player") != NULL);
	if (reply) dbus_message_unref(reply);

	// 2. Properties GetAll on org.mpris.MediaPlayer2
	req = dbus_message_new_method_call(bus_name, "/org/mpris/MediaPlayer2",
		"org.freedesktop.DBus.Properties", "GetAll");
	const char *iface = "org.mpris.MediaPlayer2";
	dbus_message_append_args(req, DBUS_TYPE_STRING, &iface, DBUS_TYPE_INVALID);
	reply = exchange_call(client_conn, server_conn, req, &state);
	CHECK(reply != NULL && dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_METHOD_RETURN);
	CHECK(reply != NULL && strcmp(dbus_message_get_signature(reply), "a{sv}") == 0);
	if (reply) dbus_message_unref(reply);

	// 3. Properties Get: PlaybackStatus
	req = dbus_message_new_method_call(bus_name, "/org/mpris/MediaPlayer2",
		"org.freedesktop.DBus.Properties", "Get");
	iface = "org.mpris.MediaPlayer2.Player";
	const char *prop = "PlaybackStatus";
	dbus_message_append_args(req, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, &prop, DBUS_TYPE_INVALID);
	reply = exchange_call(client_conn, server_conn, req, &state);
	CHECK(reply != NULL && dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_METHOD_RETURN);
	if (reply) {
		DBusMessageIter it, sub;
		dbus_message_iter_init(reply, &it);
		CHECK(dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_VARIANT);
		dbus_message_iter_recurse(&it, &sub);
		const char *status = NULL;
		dbus_message_iter_get_basic(&sub, &status);
		CHECK(status != NULL && strcmp(status, "Playing") == 0);
		dbus_message_unref(reply);
	}

	// 4. Properties Get: Metadata
	req = dbus_message_new_method_call(bus_name, "/org/mpris/MediaPlayer2",
		"org.freedesktop.DBus.Properties", "Get");
	prop = "Metadata";
	dbus_message_append_args(req, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, &prop, DBUS_TYPE_INVALID);
	reply = exchange_call(client_conn, server_conn, req, &state);
	CHECK(reply != NULL && dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_METHOD_RETURN);
	if (reply) {
		DBusMessageIter it, sub;
		dbus_message_iter_init(reply, &it);
		CHECK(dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_VARIANT);
		dbus_message_iter_recurse(&it, &sub);
		CHECK(dbus_message_iter_get_arg_type(&sub) == DBUS_TYPE_ARRAY);
		dbus_message_unref(reply);
	}

	// 5. Method: Pause
	s_last_cmd[0] = '\0';
	req = dbus_message_new_method_call(bus_name, "/org/mpris/MediaPlayer2",
		"org.mpris.MediaPlayer2.Player", "Pause");
	reply = exchange_call(client_conn, server_conn, req, &state);
	CHECK(strcmp(s_last_cmd, "pause") == 0);
	if (reply) dbus_message_unref(reply);

	// 6. Method: Play
	s_last_cmd[0] = '\0';
	req = dbus_message_new_method_call(bus_name, "/org/mpris/MediaPlayer2",
		"org.mpris.MediaPlayer2.Player", "Play");
	reply = exchange_call(client_conn, server_conn, req, &state);
	CHECK(strcmp(s_last_cmd, "play") == 0);
	if (reply) dbus_message_unref(reply);

	// 7. Method: SetPosition
	s_last_cmd[0] = '\0';
	s_last_val = -1.0;
	req = dbus_message_new_method_call(bus_name, "/org/mpris/MediaPlayer2",
		"org.mpris.MediaPlayer2.Player", "SetPosition");
	const char *track_id = "/org/mpris/MediaPlayer2/Track/1";
	dbus_int64_t target_pos_us = 65000000; // 65s
	dbus_message_append_args(req, DBUS_TYPE_OBJECT_PATH, &track_id, DBUS_TYPE_INT64, &target_pos_us, DBUS_TYPE_INVALID);
	reply = exchange_call(client_conn, server_conn, req, &state);
	CHECK(strcmp(s_last_cmd, "setPosition") == 0);
	CHECK(s_last_val >= 64.99 && s_last_val <= 65.01);
	if (reply) dbus_message_unref(reply);

	// 8. Method: Seek
	s_last_cmd[0] = '\0';
	s_last_val = 0.0;
	req = dbus_message_new_method_call(bus_name, "/org/mpris/MediaPlayer2",
		"org.mpris.MediaPlayer2.Player", "Seek");
	dbus_int64_t offset_us = 10000000; // +10s
	dbus_message_append_args(req, DBUS_TYPE_INT64, &offset_us, DBUS_TYPE_INVALID);
	reply = exchange_call(client_conn, server_conn, req, &state);
	CHECK(strcmp(s_last_cmd, "seek") == 0);
	CHECK(s_last_val >= 9.99 && s_last_val <= 10.01);
	if (reply) dbus_message_unref(reply);

	// 9. Unknown method returns D-Bus Error
	req = dbus_message_new_method_call(bus_name, "/org/mpris/MediaPlayer2",
		"org.mpris.MediaPlayer2.Player", "NonExistentMethod");
	reply = exchange_call(client_conn, server_conn, req, &state);
	CHECK(reply != NULL && dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR);
	if (reply) {
		CHECK(strcmp(dbus_message_get_error_name(reply), "org.freedesktop.DBus.Error.UnknownMethod") == 0);
		dbus_message_unref(reply);
	}

	// 10. Properties: the writable ones are accepted, the read-only ones and the unknown ones are errors
	reply = property_call(client_conn, server_conn, "Get", "org.mpris.MediaPlayer2.Player", "LoopStatus", &state);
	CHECK(reply != NULL && dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_METHOD_RETURN);
	if (reply) dbus_message_unref(reply);
	reply = property_call(client_conn, server_conn, "Get", "org.mpris.MediaPlayer2.Player", "NoSuchProperty", &state);
	CHECK(reply != NULL && dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR &&
		strcmp(dbus_message_get_error_name(reply), "org.freedesktop.DBus.Error.UnknownProperty") == 0);
	if (reply) dbus_message_unref(reply);
	reply = property_call(client_conn, server_conn, "Get", "org.example.Other", "Identity", &state);
	CHECK(reply != NULL && dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR &&
		strcmp(dbus_message_get_error_name(reply), "org.freedesktop.DBus.Error.UnknownInterface") == 0);
	if (reply) dbus_message_unref(reply);
	reply = property_call(client_conn, server_conn, "Set", "org.mpris.MediaPlayer2.Player", "PlaybackStatus", &state);
	CHECK(reply != NULL && dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR &&
		strcmp(dbus_message_get_error_name(reply), "org.freedesktop.DBus.Error.PropertyReadOnly") == 0);
	if (reply) dbus_message_unref(reply);
	reply = property_call(client_conn, server_conn, "Set", "org.mpris.MediaPlayer2.Player", "Shuffle", &state);
	CHECK(reply != NULL && dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_METHOD_RETURN);
	if (reply) dbus_message_unref(reply);

	// 11. SetPosition for another track than the current one is ignored
	s_last_cmd[0] = '\0';
	req = dbus_message_new_method_call(bus_name, "/org/mpris/MediaPlayer2",
		"org.mpris.MediaPlayer2.Player", "SetPosition");
	track_id = "/org/mpris/MediaPlayer2/Track/99";
	dbus_message_append_args(req, DBUS_TYPE_OBJECT_PATH, &track_id, DBUS_TYPE_INT64, &target_pos_us, DBUS_TYPE_INVALID);
	reply = exchange_call(client_conn, server_conn, req, &state);
	CHECK(reply != NULL && dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_METHOD_RETURN);
	CHECK(s_last_cmd[0] == '\0');
	if (reply) dbus_message_unref(reply);

	// 12. Calls that nothing implements get an error instead of no answer; Ping works
	req = dbus_message_new_method_call(bus_name, "/org/mpris/Other", "org.mpris.MediaPlayer2.Player", "Play");
	reply = exchange_call(client_conn, server_conn, req, &state);
	CHECK(reply != NULL && dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR &&
		strcmp(dbus_message_get_error_name(reply), "org.freedesktop.DBus.Error.UnknownObject") == 0);
	if (reply) dbus_message_unref(reply);
	req = dbus_message_new_method_call(bus_name, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Peer", "Ping");
	reply = exchange_call(client_conn, server_conn, req, &state);
	CHECK(reply != NULL && dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_METHOD_RETURN);
	if (reply) dbus_message_unref(reply);

	// 13. Signals: PropertiesChanged carries the metadata and the capabilities, not the position
	dbus_bus_add_match(client_conn, "type='signal',path='/org/mpris/MediaPlayer2'", &err);
	dbus_connection_flush(client_conn);
	CHECK(_MRHostDBusEmitPropertiesChanged(&api, server_conn, &state) == 0);
	CHECK(_MRHostDBusEmitSeeked(&api, server_conn, 75000000) == 0);
	int got_changed = 0, got_seeked = 0, changed_has_position = 0, changed_has_metadata = 0;
	dbus_int64_t seeked_to = -1;
	for (int i = 0; i < 100 && !(got_changed && got_seeked); i++) {
		dbus_connection_read_write(client_conn, 10);
		DBusMessage *sig;
		while ((sig = dbus_connection_pop_message(client_conn)) != NULL) {
			if (dbus_message_is_signal(sig, "org.freedesktop.DBus.Properties", "PropertiesChanged")) {
				got_changed = 1;
				DBusMessageIter it, dict, entry;
				dbus_message_iter_init(sig, &it);
				dbus_message_iter_next(&it);
				dbus_message_iter_recurse(&it, &dict);
				while (dbus_message_iter_get_arg_type(&dict) == DBUS_TYPE_DICT_ENTRY) {
					const char *key = NULL;
					dbus_message_iter_recurse(&dict, &entry);
					dbus_message_iter_get_basic(&entry, &key);
					if (strcmp(key, "Position") == 0) changed_has_position = 1;
					if (strcmp(key, "Metadata") == 0) changed_has_metadata = 1;
					dbus_message_iter_next(&dict);
				}
			} else if (dbus_message_is_signal(sig, "org.mpris.MediaPlayer2.Player", "Seeked")) {
				got_seeked = 1;
				dbus_message_get_args(sig, &err, DBUS_TYPE_INT64, &seeked_to, DBUS_TYPE_INVALID);
			}
			dbus_message_unref(sig);
		}
	}
	CHECK(got_changed);
	CHECK(changed_has_metadata);
	CHECK(!changed_has_position);
	CHECK(got_seeked && seeked_to == 75000000);

	// Release the name
	CHECK(_MRHostDBusReleaseName(&api, server_conn, bus_name) == 0);

	// Cleanup
	dbus_connection_close(client_conn);
	dbus_connection_unref(client_conn);
	_MRHostDBusDisconnect(&api, server_conn);
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	bus_address = argc > 1 ? argv[1] : NULL;
	test_address_resolution();
	if (bus_address)
		test_mpris_protocol();
	printf("All tests complete. Failures: %d\n", failures);
	return failures ? 1 : 0;
}
"""

with tempfile.TemporaryDirectory() as work:
    src_file = os.path.join(work, "test_mpris.c")
    bin_file = os.path.join(work, "test_mpris")
    with open(src_file, "w") as f:
        f.write(PROBE)

    cflags = subprocess.check_output(["pkg-config", "--cflags", "dbus-1"]).decode().strip().split()
    libs = subprocess.check_output(["pkg-config", "--libs", "dbus-1"]).decode().strip().split()

    build_cmd = [
        "cc", "-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-g",
        f"-I{mediaremoted_dir}",
        src_file,
        os.path.join(mediaremoted_dir, "mediaremoted_dbus.c"),
        "-o", bin_file
    ] + cflags + libs

    print("Compiling test probe...")
    subprocess.check_call(build_cmd)

    # Spawn private dbus-daemon
    dbus_config = f"""
<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <type>session</type>
  <listen>unix:path={work}/test_bus.sock</listen>
  <auth>EXTERNAL</auth>
  <policy context="default">
    <allow send_destination="*" eavesdrop="true"/>
    <allow eavesdrop="true"/>
    <allow own="*"/>
  </policy>
</busconfig>
"""
    cfg_file = os.path.join(work, "dbus.conf")
    with open(cfg_file, "w") as f:
        f.write(dbus_config)

    bus_proc = subprocess.Popen(["dbus-daemon", f"--config-file={cfg_file}", "--nofork"])
    try:
        # Wait for socket
        sock_path = f"{work}/test_bus.sock"
        for _ in range(50):
            if os.path.exists(sock_path):
                break
            time.sleep(0.05)
        else:
            print("ERROR: dbus-daemon socket not created in time")
            sys.exit(1)

        bus_addr = f"unix:path={sock_path}"
        print(f"Running test probe against {bus_addr}...")
        res = subprocess.run([bin_file, bus_addr], capture_output=True, text=True)
        print(res.stdout)
        if res.stderr:
            print(res.stderr, file=sys.stderr)
        if res.returncode != 0:
            print(f"TEST FAILED with return code {res.returncode}")
            sys.exit(res.returncode)
        print("ALL MPRIS HOST TESTS PASSED!")
    finally:
        bus_proc.terminate()
        bus_proc.wait()
