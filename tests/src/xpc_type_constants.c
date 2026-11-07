// Copyright (C) 2026 Lubos Dolezel
//
// XPC_TYPE_* constants (xpc/xpc.h): xpc_get_type() of an object returns the
// constant of its type, the constants are distinct, the static objects
// (XPC_BOOL_TRUE, XPC_ERROR_*) are usable, and objects survive a round trip
// through a connection. Build for x86_64 and i386.
#include <xpc/xpc.h>
#include <dispatch/dispatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

static int g_failures;

#define CHECK(cond, ...) do { \
	if (!(cond)) { printf("FAIL: " __VA_ARGS__); printf("\n"); g_failures++; } \
} while (0)

struct typed { const char* what; xpc_object_t object; xpc_type_t type; };

static void check_types(void)
{
	static const uint8_t bytes[] = { 1, 2, 3 };
	static const uuid_t uuid = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
	xpc_object_t empty_array = xpc_array_create(NULL, 0);
	xpc_object_t empty_dict = xpc_dictionary_create(NULL, NULL, 0);
	xpc_connection_t listener = xpc_connection_create(NULL, NULL);
	struct typed objects[] = {
		{ "null", xpc_null_create(), XPC_TYPE_NULL },
		{ "bool", xpc_bool_create(true), XPC_TYPE_BOOL },
		{ "int64", xpc_int64_create(-5), XPC_TYPE_INT64 },
		{ "uint64", xpc_uint64_create(5), XPC_TYPE_UINT64 },
		{ "double", xpc_double_create(1.5), XPC_TYPE_DOUBLE },
		{ "date", xpc_date_create(1000), XPC_TYPE_DATE },
		{ "data", xpc_data_create(bytes, sizeof(bytes)), XPC_TYPE_DATA },
		{ "string", xpc_string_create("x"), XPC_TYPE_STRING },
		{ "uuid", xpc_uuid_create(uuid), XPC_TYPE_UUID },
		{ "fd", xpc_fd_create(0), XPC_TYPE_FD },
		{ "array", empty_array, XPC_TYPE_ARRAY },
		{ "dictionary", empty_dict, XPC_TYPE_DICTIONARY },
		{ "error", (xpc_object_t) XPC_ERROR_CONNECTION_INVALID, XPC_TYPE_ERROR },
		{ "connection", listener, XPC_TYPE_CONNECTION },
		{ "endpoint", xpc_endpoint_create(listener), XPC_TYPE_ENDPOINT },
	};
	size_t count = sizeof(objects) / sizeof(objects[0]);

	for (size_t i = 0; i < count; i++)
	{
		CHECK(objects[i].type != NULL, "XPC_TYPE_* for %s is NULL", objects[i].what);
		CHECK(objects[i].object != NULL, "cannot create a %s", objects[i].what);
		if (objects[i].object != NULL)
			CHECK(xpc_get_type(objects[i].object) == objects[i].type,
				"xpc_get_type(%s) is %p, not its XPC_TYPE_* %p", objects[i].what,
				(void*) xpc_get_type(objects[i].object), (void*) objects[i].type);
		for (size_t j = 0; j < i; j++)
			CHECK(objects[i].type != objects[j].type, "XPC_TYPE_* for %s and %s are the same",
				objects[i].what, objects[j].what);
	}
	if (g_failures == 0)
		printf("ok: xpc_get_type matches the distinct XPC_TYPE_* of %zu types\n", count);
}

static void check_static_objects(void)
{
	int before = g_failures;
	CHECK(xpc_get_type(XPC_BOOL_TRUE) == XPC_TYPE_BOOL, "xpc_get_type(XPC_BOOL_TRUE)");
	CHECK(xpc_bool_get_value(XPC_BOOL_TRUE) == true, "XPC_BOOL_TRUE is not true");
	CHECK(xpc_bool_get_value(XPC_BOOL_FALSE) == false, "XPC_BOOL_FALSE is not false");
	CHECK(xpc_equal(XPC_BOOL_TRUE, xpc_bool_create(true)), "XPC_BOOL_TRUE != xpc_bool_create(true)");
	xpc_retain(XPC_BOOL_TRUE);
	xpc_release(XPC_BOOL_TRUE);
	char* desc = xpc_copy_description(XPC_BOOL_TRUE);
	CHECK(desc != NULL, "no description of XPC_BOOL_TRUE");
	free(desc);

	const char* text = xpc_dictionary_get_string(XPC_ERROR_CONNECTION_INVALID, XPC_ERROR_KEY_DESCRIPTION);
	CHECK(text != NULL && *text != '\0', "XPC_ERROR_CONNECTION_INVALID has no description");
	if (g_failures == before)
		printf("ok: XPC_BOOL_TRUE/FALSE and XPC_ERROR_CONNECTION_INVALID are usable\n");
}

static void describe(const char* what, xpc_object_t object)
{
	char* desc = object ? xpc_copy_description(object) : NULL;
	printf("note: %s: %s\n", what, desc ? desc : "(null)");
	fflush(stdout);
	free(desc);
}

static volatile int g_server_peers, g_server_requests, g_server_replies;

static void check_round_trip(void)
{
	int before = g_failures;
	dispatch_queue_t q = dispatch_queue_create("server", DISPATCH_QUEUE_SERIAL);
	xpc_connection_t listener = xpc_connection_create(NULL, q);
	xpc_connection_set_event_handler(listener, ^(xpc_object_t peer) {
		if (xpc_get_type(peer) != XPC_TYPE_CONNECTION) {
			describe("listener event", peer);
			return;
		}
		__atomic_add_fetch(&g_server_peers, 1, __ATOMIC_SEQ_CST);
		xpc_connection_set_event_handler(peer, ^(xpc_object_t msg) {
			if (xpc_get_type(msg) != XPC_TYPE_DICTIONARY) {
				describe("server peer event", msg);
				return;
			}
			__atomic_add_fetch(&g_server_requests, 1, __ATOMIC_SEQ_CST);
			xpc_object_t reply = xpc_dictionary_create_reply(msg);
			if (reply == NULL) {
				printf("note: the request arrived without a reply port\n");
				return;
			}
			xpc_dictionary_apply(msg, ^bool(const char* key, xpc_object_t value) {
				xpc_dictionary_set_value(reply, key, value);
				return true;
			});
			xpc_connection_send_message(peer, reply);
			__atomic_add_fetch(&g_server_replies, 1, __ATOMIC_SEQ_CST);
			xpc_release(reply);
		});
		xpc_connection_resume(peer);
	});
	xpc_connection_resume(listener);

	xpc_endpoint_t endpoint = xpc_endpoint_create(listener);
	xpc_connection_t client = xpc_connection_create_from_endpoint(endpoint);
	xpc_connection_set_event_handler(client, ^(xpc_object_t e) { describe("client event", e); });
	xpc_connection_resume(client);

	static const uuid_t uuid = { 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
	xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_bool(msg, "bool", true);
	xpc_dictionary_set_int64(msg, "int64", -42);
	xpc_dictionary_set_uint64(msg, "uint64", 42);
	xpc_dictionary_set_double(msg, "double", 2.5);
	xpc_dictionary_set_string(msg, "string", "hello");
	xpc_dictionary_set_data(msg, "data", "abc", 3);
	xpc_dictionary_set_uuid(msg, "uuid", uuid);
	xpc_dictionary_set_value(msg, "date", xpc_date_create(12345));
	xpc_dictionary_set_value(msg, "null", xpc_null_create());
	xpc_object_t array = xpc_array_create(NULL, 0);
	xpc_array_set_string(array, XPC_ARRAY_APPEND, "item");
	xpc_array_append_value(array, xpc_int64_create(7));
	xpc_dictionary_set_value(msg, "array", array);
	xpc_object_t inner = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(inner, "k", "v");
	xpc_dictionary_set_value(msg, "dict", inner);

	xpc_object_t reply = xpc_connection_send_message_with_reply_sync(client, msg);
	if (reply == NULL || xpc_get_type(reply) != XPC_TYPE_DICTIONARY)
	{
		describe("reply", reply);
		printf("note: server side: %d peer(s), %d request(s), %d reply(ies) sent\n",
			g_server_peers, g_server_requests, g_server_replies);
	}
	CHECK(reply != NULL && xpc_get_type(reply) == XPC_TYPE_DICTIONARY, "no dictionary reply over the connection");
	if (reply != NULL && xpc_get_type(reply) == XPC_TYPE_DICTIONARY)
	{
		size_t len = 0;
		const void* data = xpc_dictionary_get_data(reply, "data", &len);
		const uint8_t* ruuid = xpc_dictionary_get_uuid(reply, "uuid");
		xpc_object_t rarray = xpc_dictionary_get_value(reply, "array");
		xpc_object_t rinner = xpc_dictionary_get_value(reply, "dict");
		CHECK(xpc_dictionary_get_bool(reply, "bool") == true, "bool");
		CHECK(xpc_dictionary_get_int64(reply, "int64") == -42, "int64");
		CHECK(xpc_dictionary_get_uint64(reply, "uint64") == 42, "uint64");
		CHECK(xpc_dictionary_get_double(reply, "double") == 2.5, "double");
		CHECK(xpc_dictionary_get_string(reply, "string") && strcmp(xpc_dictionary_get_string(reply, "string"), "hello") == 0, "string");
		CHECK(data && len == 3 && memcmp(data, "abc", 3) == 0, "data");
		CHECK(ruuid && memcmp(ruuid, uuid, sizeof(uuid_t)) == 0, "uuid");
		CHECK(xpc_get_type(xpc_dictionary_get_value(reply, "date")) == XPC_TYPE_DATE
			&& xpc_date_get_value(xpc_dictionary_get_value(reply, "date")) == 12345, "date");
		CHECK(xpc_get_type(xpc_dictionary_get_value(reply, "null")) == XPC_TYPE_NULL, "null");
		CHECK(rarray && xpc_get_type(rarray) == XPC_TYPE_ARRAY && xpc_array_get_count(rarray) == 2
			&& strcmp(xpc_array_get_string(rarray, 0), "item") == 0 && xpc_array_get_int64(rarray, 1) == 7, "array");
		CHECK(rinner && xpc_get_type(rinner) == XPC_TYPE_DICTIONARY && xpc_dictionary_get_string(rinner, "k")
			&& strcmp(xpc_dictionary_get_string(rinner, "k"), "v") == 0, "nested dictionary");
	}
	if (g_failures == before)
		printf("ok: a dictionary of every basic type survives a connection round trip\n");
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);
	check_types();
	check_static_objects();
	check_round_trip();
	printf("%s\n", g_failures ? "FAIL" : "PASS");
	fflush(stdout);
	_exit(g_failures ? 1 : 0);
}
