/**
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// The com.apple.runningboard Mach service, as far as Darling needs it.
//
// Darling has no process-lifecycle management (no suspension, no jetsam-style policy), so the service has nothing to grant
// or enforce, and it enforces nothing. It owns the Mach service name so the launchd lookup succeeds, accepts
// connections, and answers, as observed on macOS 26.7.3 x86_64:
//   - handshakeWithRequest: and acquireAssertionWithDescriptor:error: with replies laid out as observed (handshake.c,
//     acquire.c); acquiring with no attributes is an error, RBSAssertionErrorDomain code 2;
//   - an operation it does not implement with an error reply, RBSServiceErrorDomain code 501 ("The server has no
//     implementation for this message"), as an archived NSError (archive.m);
//   - a request without an operation name by closing the connection (the client sees "Connection interrupted").
// Messages that expect no reply (the opening empty message, async_* operations) are ignored.
// launchd starts the daemon on the first contact (the plist has no RunAtLoad), so processes that never use the service
// never start it. The daemon keeps no per-client state.
// Test: tests/runningboardd-service.c

#include <dispatch/dispatch.h>
#include <xpc/xpc.h>
#include <xpc/connection.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "acquire.h"
#include "archive.h"
#include "handshake.h"

#define SERVICE_NAME "com.apple.runningboard"
#define SELECTOR_KEY "rbs_selector"

static bool has_attributes(xpc_object_t message) {
	xpc_object_t descriptor = xpc_dictionary_get_value(message, ACQUIRE_DESCRIPTOR_KEY);
	if (descriptor == NULL || xpc_get_type(descriptor) != XPC_TYPE_DICTIONARY) {
		return false;
	}
	xpc_object_t attributes = xpc_dictionary_get_value(descriptor, ACQUIRE_ATTRIBUTES_KEY);
	return attributes != NULL && xpc_get_type(attributes) == XPC_TYPE_ARRAY && xpc_array_get_count(attributes) > 0;
}

static void reply_to(xpc_connection_t connection, xpc_object_t message) {
	xpc_object_t reply = xpc_dictionary_create_reply(message);
	if (reply == NULL) {
		// a send-only message (the opening empty dictionary, the async_* operations): nobody is waiting for an answer
		return;
	}

	const char* selector = xpc_dictionary_get_string(message, SELECTOR_KEY);
	if (selector == NULL) {
		xpc_release(reply);
		xpc_connection_cancel(connection);
		return;
	}

	xpc_object_t value = NULL;
	if (strcmp(selector, HANDSHAKE_SELECTOR) == 0) {
		value = handshake_response_create(connection);
	} else if (strcmp(selector, ACQUIRE_SELECTOR) == 0) {
		if (has_attributes(message)) {
			value = acquire_response_create(connection);
		} else {
			archive_error_reply(reply, "RBSAssertionErrorDomain", 2, "Cannot acquire an assertion with no attributes");
		}
	} else {
		archive_error_reply(reply, "RBSServiceErrorDomain", 501, "The server has no implementation for this message");
	}

	if (value != NULL) {
		xpc_dictionary_set_value(reply, "return-value", value);
		xpc_release(value);
	}
	xpc_connection_send_message(connection, reply);
	xpc_release(reply);
}

static void handle_peer_event(xpc_connection_t connection, xpc_object_t event) {
	if (xpc_get_type(event) == XPC_TYPE_DICTIONARY) {
		reply_to(connection, event);
	}
	// connection errors (the peer went away) need no action: the daemon keeps no per-client state
}

int main(void) {
	dispatch_queue_t queue = dispatch_queue_create("org.darlinghq.runningboardd.connections", DISPATCH_QUEUE_CONCURRENT);
	xpc_connection_t server = xpc_connection_create_mach_service(SERVICE_NAME, queue, XPC_CONNECTION_MACH_SERVICE_LISTENER);
	if (server == NULL) {
		return 1;
	}

	xpc_connection_set_event_handler(server, ^(xpc_object_t event) {
		if (xpc_get_type(event) == XPC_TYPE_CONNECTION) {
			xpc_connection_t peer = event;
			xpc_connection_set_target_queue(peer, queue);
			xpc_connection_set_event_handler(peer, ^(xpc_object_t peer_event) {
				handle_peer_event(peer, peer_event);
			});
			xpc_connection_resume(peer);
		} else if (event == XPC_ERROR_CONNECTION_INVALID) {
			// the service name is gone (launchd shutting down): nothing left to serve
			exit(0);
		}
	});
	xpc_connection_resume(server);

	dispatch_main();
}
