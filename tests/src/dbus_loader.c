// Copyright (C) 2026 Lubos Dolezel
//
// The shared loader of the dbus wrapper library (src/libdarlingdbus): it
// reports a library that cannot be loaded (-1) and a library without the
// libdbus functions (-2) without touching the caller's table, hands the same
// table to every thread, and, where the wrapper is usable, the functions work
// (escaping, and a method call on a private bus when DBUS_LOADER_TEST_BUS is
// set to its address).
//
// Build the loader source into the test, with src/libdarlingdbus/include and
// the dbus-1 headers in the include path:
//   cc -o t tests/src/dbus_loader.c src/libdarlingdbus/darling_dbus.c \
//       -Isrc/libdarlingdbus/include $(pkg-config --cflags dbus-1)
#include "darling_dbus.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static const struct darling_dbus* g_seen[8];

static void* get_thread(void* arg)
{
	g_seen[(long) arg] = darling_dbus_get(0);
	return NULL;
}

static int table_is_empty(const struct darling_dbus* table)
{
	static const struct darling_dbus empty;
	return memcmp(table, &empty, sizeof(empty)) == 0;
}

static void test_failures(void)
{
	struct darling_dbus table;

	memset(&table, 0, sizeof(table));
	CHECK(darling_dbus_load("/nonexistent/libdbus.dylib", 0, &table) == -1);
	CHECK(table_is_empty(&table));

	// a library that loads but has no libdbus functions
	CHECK(darling_dbus_load("/usr/lib/libSystem.B.dylib", 0, &table) == -2);
	CHECK(table_is_empty(&table));

	// the host fallback only adds library names: inside the prefix none of them loads
	CHECK(darling_dbus_load("/nonexistent/libdbus.dylib", DARLING_DBUS_HOST_FALLBACK, &table) == -1);
	CHECK(table_is_empty(&table));
}

static void test_shared_table(void)
{
	pthread_t threads[8];
	struct darling_dbus direct;
	int usable;

	memset(&direct, 0, sizeof(direct));
	usable = darling_dbus_load(NULL, 0, &direct) == 0;

	for (long i = 0; i < 8; i++)
		CHECK(pthread_create(&threads[i], NULL, get_thread, (void*) i) == 0);
	for (int i = 0; i < 8; i++)
		pthread_join(threads[i], NULL);

	for (int i = 1; i < 8; i++)
		CHECK(g_seen[i] == g_seen[0]);
	CHECK(darling_dbus_get(0) == g_seen[0]);
	CHECK(darling_dbus_get(DARLING_DBUS_HOST_FALLBACK) == g_seen[0]);
	// the shared loader finds the wrapper exactly when a direct load of it succeeds
	CHECK((g_seen[0] != NULL) == usable);
	printf("wrapper %s\n", usable ? "usable" : "not usable");

	if (g_seen[0] == NULL)
		return;

	char* escaped = g_seen[0]->address_escape_value("a b");
	CHECK(escaped != NULL && strcmp(escaped, "a%20b") == 0);
	g_seen[0]->free(escaped);

	DBusError error;
	g_seen[0]->error_init(&error);
	DBusConnection* connection = g_seen[0]->connection_open_private("unix:path=/nonexistent/bus-socket", &error);
	CHECK(connection == NULL);
	CHECK(error.name != NULL);
	g_seen[0]->error_free(&error);
}

static void test_method_call(const struct darling_dbus* api, const char* address)
{
	DBusError error;
	api->error_init(&error);
	DBusConnection* connection = api->connection_open_private(address, &error);
	CHECK(connection != NULL);
	if (connection == NULL)
		return;
	CHECK(api->bus_register(connection, &error));

	DBusMessage* call = api->message_new_method_call("org.freedesktop.DBus", "/org/freedesktop/DBus",
		"org.freedesktop.DBus", "ListNames");
	CHECK(call != NULL);
	DBusMessage* reply = api->connection_send_with_reply_and_block(connection, call, 5000, &error);
	CHECK(reply != NULL);
	int found = 0;
	if (reply != NULL)
	{
		DBusMessageIter iter, names;
		CHECK(api->message_iter_init(reply, &iter));
		CHECK(api->message_iter_get_arg_type(&iter) == DBUS_TYPE_ARRAY);
		api->message_iter_recurse(&iter, &names);
		while (api->message_iter_get_arg_type(&names) == DBUS_TYPE_STRING)
		{
			const char* name = NULL;
			api->message_iter_get_basic(&names, &name);
			if (name != NULL && strcmp(name, "org.freedesktop.DBus") == 0)
				found = 1;
			api->message_iter_next(&names);
		}
		api->message_unref(reply);
	}
	CHECK(found);
	api->message_unref(call);
	api->connection_close(connection);
	api->connection_unref(connection);
	api->error_free(&error);
}

int main(void)
{
	test_failures();
	test_shared_table();

	const char* bus = getenv("DBUS_LOADER_TEST_BUS");
	if (bus != NULL && g_seen[0] != NULL)
		test_method_call(g_seen[0], bus);

	if (g_failures == 0)
		printf("PASS\n");
	else
		printf("FAILED %d\n", g_failures);
	return g_failures == 0 ? 0 : 1;
}
