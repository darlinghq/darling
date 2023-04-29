/**
 * This file is part of Darling.
 *
 * Copyright (C) 2023 Darling developers
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

#include <xpc/xpc.h>
#include <xpc/connection.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVICE_NAME "com.apple.coresymbolicationd"

#ifndef ENABLE_LOG
	#define ENABLE_LOG 0
#endif

#if ENABLE_LOG
	#define server_log(format, ...) do { printf("server connection: " format "\n", ## __VA_ARGS__); fflush(stdout); } while (0)
	#define server_error(format, ...) do { fprintf(stderr, "server connection: " format "\n", ## __VA_ARGS__); fflush(stderr); } while (0)
#else
	#define server_log(format, ...)
	#define server_error(format, ...)
#endif

static void connection_handler(xpc_connection_t connection) {
	server_log("Got a new connection: %p\n", connection);

	xpc_connection_set_event_handler(connection, ^(xpc_object_t object) {
		char* desc = xpc_copy_description(object);
		xpc_type_t type = xpc_get_type(object);
		server_log("Received %s from connection %p: %s\n", (type == (xpc_type_t)XPC_TYPE_DICTIONARY) ? "message" : ((type == (xpc_type_t)XPC_TYPE_ERROR) ? "error" : "unexpected object"), connection, desc);
		free(desc);

		// stub reply
		xpc_object_t reply = xpc_dictionary_create_reply(object);
		xpc_connection_send_message(connection, reply);
	});
	xpc_connection_resume(connection);
};

static void handle_server_error(xpc_object_t error) {
	if (error == XPC_ERROR_CONNECTION_INVALID) {
		server_log("server died (or got cancelled)\n");
	} else if (error == XPC_ERROR_TERMINATION_IMMINENT) {
		server_log("someone wants to kill us\n");
	} else {
		server_error("received unexpected error: %s\n", xpc_copy_description(error));
		exit(1);
	}
};

int main(int arc, char** argv) {
	xpc_connection_t server = NULL;

	server = xpc_connection_create_mach_service(SERVICE_NAME, NULL, XPC_CONNECTION_MACH_SERVICE_LISTENER);

	xpc_connection_set_event_handler(server, ^(xpc_object_t object) {
		xpc_type_t obj_type = xpc_get_type(object);
		if (obj_type == (xpc_type_t)XPC_TYPE_CONNECTION) {
			connection_handler(object);
		} else if (obj_type == (xpc_type_t)XPC_TYPE_ERROR) {
			handle_server_error(object);
		} else {
			server_error("received non-connection, non-error object in event handler: %s\n", xpc_copy_description(object));
			exit(1);
		}
	});

	xpc_connection_resume(server);

	dispatch_main();
	return 0;
};
