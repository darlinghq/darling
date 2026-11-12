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
#include <Foundation/Foundation.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include "../LSDProtocol.h"

static void connectionCallback(xpc_connection_t connection);
static void handleMessage(xpc_connection_t connection, xpc_object_t msg);

dispatch_queue_t g_serverQueue;

@class AEReceiver, AEPendingLookup;
// Both are only touched on g_serverQueue. Newest check-ins are last.
static NSMutableArray<AEReceiver*>* g_receivers;
static NSMutableArray<AEPendingLookup*>* g_pendingLookups;

int main(int argc, const char** argv)
{
	if (argc > 1) {
		if (argc != 3 || strcmp(argv[1], "--register") != 0) {
			fprintf(stderr, "usage: %s [--register BUNDLE]\n", argv[0]);
			return 2;
		}
		@autoreleasepool {
			BOOL success = [LSBundle registerBundleAtPath:[NSString stringWithUTF8String:argv[2]]];
			if (!success) fprintf(stderr, "launchservicesd: registration failed: %s\n", argv[2]);
			return success ? 0 : 1;
		}
	}
	g_serverQueue = dispatch_queue_create("service queue", NULL);
	g_receivers = [[NSMutableArray alloc] init];
	g_pendingLookups = [[NSMutableArray alloc] init];
	
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
		// The receiver and lookup tables belong to the service queue.
		xpc_retain(msg);
		dispatch_async(g_serverQueue, ^{
			handleMessage(connection, msg);
			xpc_release(msg);
		});
	});
	xpc_connection_resume(connection);
}

// OSStatus values used in replies.
enum {
	kErrFileNotFound = -43,
	kErrProcessNotFound = -600,
	kErrNotAnApplication = -10811,
	kErrUnknown = -10810,
	kErrParameter = -50,
};

// A process that checked in to receive Apple events.
@interface AEReceiver : NSObject
{
@public
	pid_t pid;
	NSString* bundleID;
	xpc_connection_t owner; // the connection the check-in came over
	xpc_object_t endpoint;
}
@end

@implementation AEReceiver
- (void)dealloc
{
	[bundleID release];
	if (owner) xpc_release(owner);
	if (endpoint) xpc_release(endpoint);
	[super dealloc];
}
@end

// A lookup for a pid that has not checked in yet.
@interface AEPendingLookup : NSObject
{
@public
	pid_t pid;
	xpc_object_t request;
	xpc_connection_t connection;
	dispatch_time_t deadline;
}
- (void)poll;
@end

static AEReceiver* findReceiver(pid_t pid, const char* bundleID)
{
	for (AEReceiver* r in [g_receivers reverseObjectEnumerator])
	{
		if (pid > 0)
		{
			if (r->pid == pid)
				return r;
		}
		else if (bundleID != NULL && r->bundleID != nil && strcasecmp([r->bundleID UTF8String], bundleID) == 0)
		{
			// A receiver whose process is gone is not a candidate.
			if (kill(r->pid, 0) == 0 || errno == EPERM)
				return r;
		}
	}
	return nil;
}

static void sendLookupReply(xpc_object_t request, xpc_connection_t connection, AEReceiver* receiver, int err)
{
	xpc_object_t reply = xpc_dictionary_create_reply(request);
	if (reply == NULL)
		return;
	xpc_dictionary_set_int64(reply, LSD_KEY_ERR, receiver ? 0 : err);
	if (receiver)
	{
		xpc_dictionary_set_int64(reply, LSD_KEY_PID, receiver->pid);
		xpc_dictionary_set_value(reply, LSD_KEY_ENDPOINT, receiver->endpoint);
	}
	xpc_connection_send_message(connection, reply);
	xpc_release(reply);
}

@implementation AEPendingLookup
- (void)dealloc
{
	if (request) xpc_release(request);
	if (connection) xpc_release(connection);
	[super dealloc];
}

- (void)poll
{
	AEReceiver* receiver = findReceiver(pid, NULL);
	BOOL alive = kill(pid, 0) == 0 || errno == EPERM;

	if (receiver || !alive || dispatch_time(DISPATCH_TIME_NOW, 0) >= deadline)
	{
		sendLookupReply(request, connection, receiver, kErrProcessNotFound);
		[g_pendingLookups removeObject:self];
		return;
	}
	// A check-in resolves the lookup at once; the poll only notices a process that exited.
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 250 * NSEC_PER_MSEC), g_serverQueue, ^{
		if ([g_pendingLookups containsObject:self])
			[self poll];
	});
}
@end

static void dropReceiversOf(xpc_connection_t connection)
{
	for (AEReceiver* r in [[g_receivers copy] autorelease])
		if (r->owner == connection)
			[g_receivers removeObject:r];
}

static void handleRegister(xpc_connection_t connection, xpc_object_t msg)
{
	const char* path = xpc_dictionary_get_string(msg, LSD_KEY_PATH);
	int err = 0;
	struct stat st;

	if (path == NULL || path[0] == 0)
		err = kErrParameter;
	else if (stat(path, &st) != 0)
		err = kErrFileNotFound;
	else if (!S_ISDIR(st.st_mode))
		err = kErrNotAnApplication;
	else
	{
		@autoreleasepool {
			if (![LSBundle registerBundleAtPath:[NSString stringWithUTF8String:path]])
				err = kErrUnknown;
		}
	}

	xpc_object_t reply = xpc_dictionary_create_reply(msg);
	xpc_dictionary_set_int64(reply, LSD_KEY_ERR, err);
	xpc_connection_send_message(connection, reply);
	xpc_release(reply);
}

static void handleAECheckIn(xpc_connection_t connection, xpc_object_t msg)
{
	int64_t pid = xpc_dictionary_get_int64(msg, LSD_KEY_PID);
	const char* bundleID = xpc_dictionary_get_string(msg, LSD_KEY_BUNDLE_ID);
	xpc_object_t endpoint = xpc_dictionary_get_value(msg, LSD_KEY_ENDPOINT);
	int err = 0;

	if (pid <= 0 || endpoint == NULL || xpc_get_type(endpoint) != XPC_TYPE_ENDPOINT)
		err = kErrParameter;
	else
	{
		AEReceiver* r = [[[AEReceiver alloc] init] autorelease];
		r->pid = (pid_t) pid;
		r->bundleID = bundleID ? [[NSString stringWithUTF8String:bundleID] retain] : nil;
		r->owner = (xpc_connection_t) xpc_retain(connection);
		r->endpoint = xpc_retain(endpoint);

		// A process checks in once; a repeat replaces the earlier entry.
		for (AEReceiver* old in [[g_receivers copy] autorelease])
			if (old->pid == r->pid)
				[g_receivers removeObject:old];
		[g_receivers addObject:r];

		for (AEPendingLookup* p in [[g_pendingLookups copy] autorelease])
			if (p->pid == r->pid)
				[p poll];
	}

	xpc_object_t reply = xpc_dictionary_create_reply(msg);
	xpc_dictionary_set_int64(reply, LSD_KEY_ERR, err);
	xpc_connection_send_message(connection, reply);
	xpc_release(reply);
}

static void handleAELookup(xpc_connection_t connection, xpc_object_t msg)
{
	int64_t pid = xpc_dictionary_get_int64(msg, LSD_KEY_PID);
	const char* bundleID = xpc_dictionary_get_string(msg, LSD_KEY_BUNDLE_ID);
	int64_t waitMs = xpc_dictionary_get_int64(msg, LSD_KEY_WAIT_MS);

	if (pid <= 0 && bundleID == NULL)
	{
		sendLookupReply(msg, connection, nil, kErrParameter);
		return;
	}

	AEReceiver* receiver = findReceiver((pid_t) pid, bundleID);
	if (receiver || pid <= 0 || waitMs <= 0)
	{
		sendLookupReply(msg, connection, receiver, kErrProcessNotFound);
		return;
	}

	AEPendingLookup* p = [[[AEPendingLookup alloc] init] autorelease];
	p->pid = (pid_t) pid;
	p->request = xpc_retain(msg);
	p->connection = (xpc_connection_t) xpc_retain(connection);
	p->deadline = dispatch_time(DISPATCH_TIME_NOW, waitMs * NSEC_PER_MSEC);
	[g_pendingLookups addObject:p];
	[p poll];
}

static void handleMessage(xpc_connection_t connection, xpc_object_t msg)
{
	xpc_type_t type = xpc_get_type(msg);

	if (type == XPC_TYPE_ERROR)
	{
		// The client went away (or the connection broke): its Apple event endpoint is gone with it.
		dropReceiversOf(connection);
		return;
	}

	if (type == XPC_TYPE_DICTIONARY)
	{
		const char* op = xpc_dictionary_get_string(msg, LSD_KEY_OP);

		if (op != NULL && strcmp(op, LSD_OP_REGISTER) == 0)
			handleRegister(connection, msg);
		else if (op != NULL && strcmp(op, LSD_OP_AE_CHECKIN) == 0)
			handleAECheckIn(connection, msg);
		else if (op != NULL && strcmp(op, LSD_OP_AE_LOOKUP) == 0)
			handleAELookup(connection, msg);
		else
		{
			xpc_object_t reply = xpc_dictionary_create_reply(msg);

			xpc_connection_send_message(connection, reply);
		}
	}
}
