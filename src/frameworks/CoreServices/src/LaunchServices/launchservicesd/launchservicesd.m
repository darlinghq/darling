/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <xpc/xpc.h>
#include <dispatch/dispatch.h>
#include "LSBundle.h"

static void connectionCallback(xpc_connection_t connection);
static void handleMessage(xpc_connection_t connection, xpc_object_t msg);

dispatch_queue_t g_serverQueue;

int main(int argc, const char** argv)
{
	g_serverQueue = dispatch_queue_create("service queue", NULL);
	
	[LSBundle scanForBundles];
	[LSBundle watchForBundles];

	// xpc_main(connectionCallback);
	// Our xpc_main() isn't done yet, so we do this manually

	xpc_connection_t listener = xpc_connection_create_mach_service("com.apple.coreservices.launchservicesd",
		g_serverQueue, XPC_CONNECTION_MACH_SERVICE_LISTENER);

	xpc_connection_set_event_handler(listener, ^(xpc_object_t peer) {
		if (xpc_get_type(peer) == XPC_TYPE_CONNECTION)
			connectionCallback((xpc_connection_t) peer);
	});
	xpc_connection_resume(listener);

	dispatch_main();
	return 0;
}

static void connectionCallback(xpc_connection_t connection)
{
	pid_t pid = xpc_connection_get_pid(connection);
	xpc_connection_set_event_handler(connection, ^(xpc_object_t msg) {
		handleMessage(connection, msg);
	});
	xpc_connection_resume(connection);
}

static void handleMessage(xpc_connection_t connection, xpc_object_t msg)
{
	xpc_type_t type = xpc_get_type(msg);

	if (type == XPC_TYPE_DICTIONARY)
	{
		xpc_object_t reply = xpc_dictionary_create_reply(msg);

		xpc_connection_send_message(connection, reply);
	}
}
