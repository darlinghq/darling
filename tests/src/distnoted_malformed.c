// Copyright (C) 2026 Lubos Dolezel
//
// The distributed notification daemon survives requests with the wrong types, missing or oversized arguments, and
// keeps serving: a registration made afterwards still receives a notification, and one the daemon refused does not.
#include <CoreFoundation/CoreFoundation.h>
#include <dispatch/dispatch.h>
#include <stdio.h>
#include <string.h>
#include <xpc/xpc.h>

#define SERVICE "com.apple.distributed_notifications@Uv3"

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static xpc_connection_t connect(dispatch_queue_t queue, dispatch_semaphore_t delivered, int *deliveries)
{
	xpc_connection_t c = xpc_connection_create_mach_service(SERVICE, queue, 0);
	xpc_connection_set_event_handler(c, ^(xpc_object_t event) {
		if (xpc_get_type(event) == XPC_TYPE_DICTIONARY && xpc_dictionary_get_int64(event, "op") == 5) {
			(*deliveries)++;
			dispatch_semaphore_signal(delivered);
		}
	});
	xpc_connection_resume(c);
	return c;
}

static void send(xpc_connection_t c, xpc_object_t m)
{
	xpc_connection_send_message(c, m);
	xpc_release(m);
}

int main(void)
{
	dispatch_queue_t queue = dispatch_queue_create("malformed", NULL);
	dispatch_semaphore_t delivered = dispatch_semaphore_create(0);
	int deliveries = 0;
	xpc_connection_t c = connect(queue, delivered, &deliveries);

	xpc_object_t hello = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_int64(hello, "op", 0);
	xpc_object_t reply = xpc_connection_send_message_with_reply_sync(c, hello);
	CHECK(xpc_get_type(reply) == XPC_TYPE_DICTIONARY);
	xpc_release(hello);

	char big[5000];
	memset(big, 'x', sizeof(big) - 1);
	big[sizeof(big) - 1] = 0;

	xpc_object_t m = xpc_dictionary_create(NULL, NULL, 0);          // no op at all
	send(c, m);
	m = xpc_dictionary_create(NULL, NULL, 0);                       // op of the wrong type
	xpc_dictionary_set_string(m, "op", "add");
	send(c, m);
	m = xpc_dictionary_create(NULL, NULL, 0);                       // add without id or behavior
	xpc_dictionary_set_int64(m, "op", 1);
	send(c, m);
	m = xpc_dictionary_create(NULL, NULL, 0);                       // add with a name that is not a string
	xpc_dictionary_set_int64(m, "op", 1);
	xpc_dictionary_set_int64(m, "id", 1);
	xpc_dictionary_set_int64(m, "behavior", 4);
	xpc_dictionary_set_int64(m, "name", 7);
	send(c, m);
	m = xpc_dictionary_create(NULL, NULL, 0);                       // add with an oversized name
	xpc_dictionary_set_int64(m, "op", 1);
	xpc_dictionary_set_int64(m, "id", 2);
	xpc_dictionary_set_int64(m, "behavior", 4);
	xpc_dictionary_set_string(m, "name", big);
	send(c, m);
	m = xpc_dictionary_create(NULL, NULL, 0);                       // remove of an id that does not exist
	xpc_dictionary_set_int64(m, "op", 2);
	xpc_dictionary_set_int64(m, "id", 99);
	send(c, m);
	m = xpc_dictionary_create(NULL, NULL, 0);                       // post with a name that is not a string
	xpc_dictionary_set_int64(m, "op", 4);
	xpc_dictionary_set_int64(m, "name", 7);
	send(c, m);
	m = xpc_dictionary_create(NULL, NULL, 0);                       // post with an oversized name and one without name
	xpc_dictionary_set_int64(m, "op", 4);
	xpc_dictionary_set_string(m, "name", big);
	send(c, m);
	m = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_int64(m, "op", 4);
	send(c, m);
	m = xpc_dictionary_create(NULL, NULL, 0);                       // post with user info that is not data
	xpc_dictionary_set_int64(m, "op", 4);
	xpc_dictionary_set_string(m, "name", "malformed.test");
	xpc_dictionary_set_string(m, "userinfo", "not data");
	send(c, m);
	m = xpc_dictionary_create(NULL, NULL, 0);                       // an unknown op
	xpc_dictionary_set_int64(m, "op", 12345);
	send(c, m);

	// None of the refused registrations exist, so none of the notifications above was delivered.
	CHECK(dispatch_semaphore_wait(delivered, dispatch_time(DISPATCH_TIME_NOW, 500 * NSEC_PER_MSEC)) != 0);
	CHECK(deliveries == 0);

	// The daemon still serves: a good registration and a post from another connection.
	m = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_int64(m, "op", 1);
	xpc_dictionary_set_int64(m, "id", 3);
	xpc_dictionary_set_int64(m, "behavior", 4);
	xpc_dictionary_set_string(m, "name", "malformed.test");
	send(c, m);
	int other = 0;
	xpc_connection_t c2 = connect(queue, delivered, &other);
	m = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_int64(m, "op", 4);
	xpc_dictionary_set_string(m, "name", "malformed.test");
	send(c2, m);
	CHECK(dispatch_semaphore_wait(delivered, dispatch_time(DISPATCH_TIME_NOW, 3 * NSEC_PER_SEC)) == 0);
	CHECK(deliveries == 1);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures != 0;
}
