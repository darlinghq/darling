#include <xpc/xpc.h>
#include <FSEvents/FSEvents.h>
#include <iostream>
#include <dispatch/dispatch.h>
#include <sqlite3.h>

#define DATABASE_VERSION 1

static void connectionCallback(xpc_connection_t connection);
static void handleMessage(xpc_connection_t connection, xpc_object_t msg);

sqlite3* g_database;

static const char* MONITORED_DIRECTORIES[] = {
	"/Applications",
	"/System/Applications"
	"/System/Library/Frameworks"
};

int main(int argc, const char** argv)
{
	if (int err = sqlite3_open("/var/cache/launchservices.db", &g_database); err != SQLITE_OK)
	{
		std::cerr << "SQLite error: " << sqlite3_errstr(err) << std::endl;
		exit(1);
	}

	// xpc_main(connectionCallback);
	// Our xpc_main() isn't done yet, so we do this manually

	dispatch_queue_t server_queue = dispatch_queue_create("xpc service", NULL);

	xpc_connection_t listener = xpc_connection_create_mach_service("com.apple.coreservices.launchservicesd",
		server_queue, XPC_CONNECTION_MACH_SERVICE_LISTENER);

	xpc_connection_set_event_handler(listener, ^(xpc_object_t peer) {
		if (xpc_get_type(peer) == XPC_TYPE_CONNECTION)
			connectionCallback(xpc_connection_t(peer));
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
