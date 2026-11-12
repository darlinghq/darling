/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

// Apple event handler tables and delivery between processes.
//
// A process that installs an event handler can receive events: it opens an anonymous XPC listener
// and announces its endpoint (with its pid and bundle identifier) to launchservicesd, which keeps
// the directory of receivers. AESendMessage asks launchservicesd for the target's endpoint,
// connects to it directly and sends the flattened event. The receiver runs the handler on the
// main run loop, as Apple event handlers expect, and returns the flattened reply.

#include <AE/AE.h>
#include <CarbonCore/MacErrors.h>
#include <CoreFoundation/CoreFoundation.h>
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xpc/xpc.h>
#include "../LaunchServices/LSDProtocol.h"

typedef struct AEHandlerEntry {
	AEEventClass eventClass;
	AEEventID eventID;
	AEEventHandlerUPP handler;
	SRefCon refcon;
	Boolean system;
} AEHandlerEntry;

static pthread_mutex_t g_handlerLock = PTHREAD_MUTEX_INITIALIZER;
static AEHandlerEntry* g_handlers;
static size_t g_handlerCount, g_handlerCapacity;

static void startReceiver(void);

static AEHandlerEntry* findExact(AEEventClass eventClass, AEEventID eventID, Boolean system)
{
	for (size_t i = 0; i < g_handlerCount; i++) {
		AEHandlerEntry* e = &g_handlers[i];
		if (e->eventClass == eventClass && e->eventID == eventID && e->system == system)
			return e;
	}
	return NULL;
}

OSErr AEInstallEventHandler(AEEventClass theAEEventClass, AEEventID theAEEventID, AEEventHandlerUPP handler,
	SRefCon handlerRefcon, Boolean isSysHandler)
{
	if (handler == NULL)
		return paramErr;

	pthread_mutex_lock(&g_handlerLock);
	AEHandlerEntry* entry = findExact(theAEEventClass, theAEEventID, isSysHandler);
	if (entry == NULL) {
		if (g_handlerCount == g_handlerCapacity) {
			size_t capacity = g_handlerCapacity ? g_handlerCapacity * 2 : 8;
			AEHandlerEntry* grown = realloc(g_handlers, capacity * sizeof(*grown));
			if (grown == NULL) {
				pthread_mutex_unlock(&g_handlerLock);
				return memFullErr;
			}
			g_handlers = grown;
			g_handlerCapacity = capacity;
		}
		entry = &g_handlers[g_handlerCount++];
		entry->eventClass = theAEEventClass;
		entry->eventID = theAEEventID;
		entry->system = isSysHandler;
	}
	entry->handler = handler;
	entry->refcon = handlerRefcon;
	pthread_mutex_unlock(&g_handlerLock);

	startReceiver();
	return noErr;
}

OSErr AERemoveEventHandler(AEEventClass theAEEventClass, AEEventID theAEEventID, AEEventHandlerUPP handler, Boolean isSysHandler)
{
	OSErr err = errAEHandlerNotFound;
	pthread_mutex_lock(&g_handlerLock);
	AEHandlerEntry* entry = findExact(theAEEventClass, theAEEventID, isSysHandler);
	if (entry != NULL) {
		*entry = g_handlers[--g_handlerCount];
		err = noErr;
	}
	pthread_mutex_unlock(&g_handlerLock);
	return err;
}

OSErr AEGetEventHandler(AEEventClass theAEEventClass, AEEventID theAEEventID, AEEventHandlerUPP* handler,
	SRefCon* handlerRefcon, Boolean isSysHandler)
{
	OSErr err = errAEHandlerNotFound;
	pthread_mutex_lock(&g_handlerLock);
	AEHandlerEntry* entry = findExact(theAEEventClass, theAEEventID, isSysHandler);
	if (entry != NULL) {
		if (handler)
			*handler = entry->handler;
		if (handlerRefcon)
			*handlerRefcon = entry->refcon;
		err = noErr;
	}
	pthread_mutex_unlock(&g_handlerLock);
	return err;
}

// The handler for an event: the application's before the system's, an exact class and ID before
// wildcards.
static Boolean lookupHandler(AEEventClass eventClass, AEEventID eventID, AEHandlerEntry* out)
{
	Boolean found = false;
	pthread_mutex_lock(&g_handlerLock);
	for (int system = 0; system < 2 && !found; system++) {
		static const struct { Boolean anyClass, anyID; } order[] = { { false, false }, { false, true }, { true, false }, { true, true } };
		for (size_t o = 0; o < 4 && !found; o++) {
			AEHandlerEntry* e = findExact(order[o].anyClass ? typeWildCard : eventClass,
				order[o].anyID ? typeWildCard : eventID, system);
			if (e != NULL) {
				*out = *e;
				found = true;
			}
		}
	}
	pthread_mutex_unlock(&g_handlerLock);
	return found;
}

// Runs the handler for an event in the calling thread. The reply, if any, is filled in by the handler.
static OSErr dispatchEvent(const AppleEvent* event, AppleEvent* reply)
{
	AEEventClass eventClass = 0;
	AEEventID eventID = 0;
	DescType type;
	Size size;
	if (AEGetAttributePtr(event, keyEventClassAttr, typeType, &type, &eventClass, sizeof(eventClass), &size) != noErr ||
		AEGetAttributePtr(event, keyEventIDAttr, typeType, &type, &eventID, sizeof(eventID), &size) != noErr)
		return errAENotAppleEvent;

	AEHandlerEntry entry;
	if (!lookupHandler(eventClass, eventID, &entry))
		return errAEEventNotHandled;
	return entry.handler(event, reply, entry.refcon);
}

// --- Connection to launchservicesd ---

static xpc_connection_t lsdConnection(void)
{
	static dispatch_once_t once;
	static xpc_connection_t connection;
	dispatch_once(&once, ^{
		const char* name = getenv(LSD_SERVICE_NAME_OVERRIDE_ENV);
		connection = xpc_connection_create_mach_service(name ? name : LSD_SERVICE_NAME,
			dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), 0);
		if (connection == NULL)
			return;
		xpc_connection_set_event_handler(connection, ^(xpc_object_t event) {
			// Errors need no action: the next request fails by itself, and a request after
			// an interruption reconnects.
		});
		xpc_connection_resume(connection);
	});
	return connection;
}

static xpc_object_t lsdRequest(xpc_object_t request, int64_t timeoutMs)
{
	xpc_connection_t connection = lsdConnection();
	if (connection == NULL)
		return NULL;

	__block xpc_object_t answer = NULL;
	dispatch_semaphore_t done = dispatch_semaphore_create(0);
	xpc_connection_send_message_with_reply(connection, request, dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0),
		^(xpc_object_t reply) {
			if (xpc_get_type(reply) == XPC_TYPE_DICTIONARY)
				answer = xpc_retain(reply);
			dispatch_semaphore_signal(done);
		});
	if (dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, timeoutMs * NSEC_PER_MSEC)) != 0) {
		// The reply block may still run later: leave `answer` alone.
		return NULL;
	}
	dispatch_release(done);
	return answer;
}

// --- Receiving ---

static xpc_connection_t g_listener;

static void handleIncoming(xpc_object_t message)
{
	size_t length = 0;
	const void* data = xpc_dictionary_get_data(message, LSD_KEY_EVENT, &length);
	xpc_object_t reply = xpc_dictionary_create_reply(message);

	AppleEvent event, replyEvent;
	AEInitializeDesc(&event);
	AEInitializeDesc(&replyEvent);
	OSErr err = data ? (OSErr) AEUnflattenDesc(data, &event) : errAECorruptData;
	if (err == noErr) {
		err = AECreateAppleEvent(kCoreEventClass, kAEAnswer, NULL, kAutoGenerateReturnID, kAnyTransactionID, &replyEvent);
		if (err == noErr) {
			err = dispatchEvent(&event, &replyEvent);
			if (err != noErr) {
				SInt32 number = err;
				AEPutParamPtr(&replyEvent, keyErrorNumber, typeSInt32, &number, sizeof(number));
			}
		}
	}

	if (reply != NULL) {
		xpc_dictionary_set_int64(reply, LSD_KEY_ERR, err);
		Size size = 0;
		AEFlattenDesc(&replyEvent, NULL, 0, &size);
		if (size > 0) {
			void* flat = malloc(size);
			if (flat != NULL) {
				if (AEFlattenDesc(&replyEvent, flat, size, &size) == noErr)
					xpc_dictionary_set_data(reply, LSD_KEY_REPLY, flat, size);
				free(flat);
			}
		}
		xpc_connection_send_message(xpc_dictionary_get_remote_connection(message), reply);
		xpc_release(reply);
	}
	AEDisposeDesc(&event);
	AEDisposeDesc(&replyEvent);
}

static void announceToLaunchServices(void)
{
	xpc_object_t request = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(request, LSD_KEY_OP, LSD_OP_AE_CHECKIN);
	xpc_dictionary_set_int64(request, LSD_KEY_PID, getpid());
	CFBundleRef main = CFBundleGetMainBundle();
	CFStringRef identifier = main ? CFBundleGetIdentifier(main) : NULL;
	char buffer[512];
	if (identifier && CFStringGetCString(identifier, buffer, sizeof(buffer), kCFStringEncodingUTF8))
		xpc_dictionary_set_string(request, LSD_KEY_BUNDLE_ID, buffer);
	xpc_object_t endpoint = xpc_endpoint_create(g_listener);
	xpc_dictionary_set_value(request, LSD_KEY_ENDPOINT, endpoint);
	xpc_release(endpoint);
	xpc_object_t reply = lsdRequest(request, 10000);
	if (reply)
		xpc_release(reply);
}

static void startReceiver(void)
{
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		// Without launchservicesd (SCDontUseServer) nothing can send events to this process.
		const char* skip = getenv("SCDontUseServer");
		if (skip != NULL && atoi(skip) != 0)
			return;

		g_listener = xpc_connection_create(NULL, dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0));
		if (g_listener == NULL)
			return;
		xpc_connection_set_event_handler(g_listener, ^(xpc_object_t peer) {
			if (xpc_get_type(peer) != XPC_TYPE_CONNECTION)
				return;
			xpc_connection_set_event_handler((xpc_connection_t) peer, ^(xpc_object_t message) {
				if (xpc_get_type(message) != XPC_TYPE_DICTIONARY)
					return;
				xpc_retain(message);
				CFRunLoopRef main = CFRunLoopGetMain();
				CFRunLoopPerformBlock(main, kCFRunLoopCommonModes, ^{
					handleIncoming(message);
					xpc_release(message);
				});
				CFRunLoopWakeUp(main);
			});
			xpc_connection_resume((xpc_connection_t) peer);
		});
		xpc_connection_resume(g_listener);

		dispatch_async(dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
			announceToLaunchServices();
		});
	});
}

// --- Sending ---

static int64_t timeoutToMs(long ticks)
{
	if (ticks == kNoTimeOut)
		return 10 * 60 * 1000;
	if (ticks == kAEDefaultTimeout || ticks < 0)
		return 60 * 1000;
	return ticks * 1000 / 60;
}

OSStatus AESendMessage(const AppleEvent* event, AppleEvent* reply, AESendMode sendMode, long timeOutInTicks)
{
	if (reply)
		AEInitializeDesc(reply);
	if (event == NULL)
		return paramErr;

	AEDesc target;
	OSErr err = AEGetAttributeDesc(event, keyAddressAttr, typeWildCard, &target);
	if (err != noErr)
		return err;

	pid_t pid = 0;
	char bundleID[512] = "";
	Size size;
	if (target.descriptorType == typeKernelProcessID) {
		SInt32 value = 0;
		err = AEGetDescData(&target, &value, sizeof(value));
		pid = value;
	} else if (target.descriptorType == typeApplicationBundleID || target.descriptorType == typeUTF8Text) {
		size = AEGetDescDataSize(&target);
		if (size <= 0 || size >= (Size) sizeof(bundleID))
			err = errAEUnknownAddressType;
		else {
			err = AEGetDescData(&target, bundleID, size);
			bundleID[size] = 0;
		}
	} else {
		err = errAEUnknownAddressType;
	}
	AEDisposeDesc(&target);
	if (err != noErr)
		return err;

	int64_t timeoutMs = timeoutToMs(timeOutInTicks);
	Boolean wantReply = (sendMode & 3) == kAEWaitReply || (sendMode & 3) == kAEQueueReply;

	if (pid == getpid()) {
		// An event for this process runs its handler directly; the main run loop may be the caller.
		AppleEvent localReply;
		AEInitializeDesc(&localReply);
		err = AECreateAppleEvent(kCoreEventClass, kAEAnswer, NULL, kAutoGenerateReturnID, kAnyTransactionID, &localReply);
		if (err == noErr)
			err = dispatchEvent(event, &localReply);
		if (reply && wantReply)
			*reply = localReply;
		else
			AEDisposeDesc(&localReply);
		return err;
	}

	// Find the receiver.
	xpc_object_t lookup = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(lookup, LSD_KEY_OP, LSD_OP_AE_LOOKUP);
	if (pid > 0) {
		xpc_dictionary_set_int64(lookup, LSD_KEY_PID, pid);
		xpc_dictionary_set_int64(lookup, LSD_KEY_WAIT_MS, timeoutMs);
	} else {
		xpc_dictionary_set_string(lookup, LSD_KEY_BUNDLE_ID, bundleID);
	}
	xpc_object_t found = lsdRequest(lookup, timeoutMs + 5000);
	xpc_release(lookup);
	if (found == NULL)
		return procNotFound;
	xpc_object_t endpoint = xpc_dictionary_get_value(found, LSD_KEY_ENDPOINT);
	int64_t lookupErr = xpc_dictionary_get_int64(found, LSD_KEY_ERR);
	if (lookupErr != 0 || xpc_get_type(endpoint) != XPC_TYPE_ENDPOINT) {
		xpc_release(found);
		return lookupErr != 0 ? (OSStatus) lookupErr : procNotFound;
	}
	xpc_connection_t connection = xpc_connection_create_from_endpoint((xpc_endpoint_t) endpoint);
	xpc_release(found);
	if (connection == NULL)
		return procNotFound;
	xpc_connection_set_event_handler(connection, ^(xpc_object_t e) {});
	xpc_connection_resume(connection);

	// Deliver it.
	Size flatSize = 0;
	AEFlattenDesc(event, NULL, 0, &flatSize);
	void* flat = flatSize > 0 ? malloc(flatSize) : NULL;
	if (flat == NULL || (err = (OSErr) AEFlattenDesc(event, flat, flatSize, &flatSize)) != noErr) {
		free(flat);
		xpc_connection_cancel(connection);
		xpc_release(connection);
		return err != noErr ? err : memFullErr;
	}
	xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_data(message, LSD_KEY_EVENT, flat, flatSize);
	free(flat);

	OSStatus result = noErr;
	if (!wantReply) {
		// The connection is closed once the receiver has answered, without anyone waiting for it.
		xpc_connection_send_message_with_reply(connection, message, dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0),
			^(xpc_object_t r) {
				xpc_connection_cancel(connection);
				xpc_release(connection);
			});
	} else {
		__block xpc_object_t answer = NULL;
		dispatch_semaphore_t done = dispatch_semaphore_create(0);
		xpc_connection_send_message_with_reply(connection, message, dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0),
			^(xpc_object_t r) {
				if (xpc_get_type(r) == XPC_TYPE_DICTIONARY)
					answer = xpc_retain(r);
				dispatch_semaphore_signal(done);
			});
		if (dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, timeoutMs * NSEC_PER_MSEC)) != 0) {
			result = errAETimeout;
		} else if (answer == NULL) {
			result = procNotFound;
		} else {
			result = (OSStatus) xpc_dictionary_get_int64(answer, LSD_KEY_ERR);
			size_t length = 0;
			const void* data = xpc_dictionary_get_data(answer, LSD_KEY_REPLY, &length);
			if (data != NULL && reply != NULL)
				AEUnflattenDesc(data, reply);
			xpc_release(answer);
		}
		xpc_connection_cancel(connection);
		xpc_release(connection);
	}
	xpc_release(message);
	return result;
}

OSErr AESend(const AppleEvent* theAppleEvent, AppleEvent* reply, AESendMode sendMode, AESendPriority sendPriority,
	SInt32 timeOutInTicks, AEIdleUPP idleProc, AEFilterUPP filterProc)
{
	return (OSErr) AESendMessage(theAppleEvent, reply, sendMode, timeOutInTicks);
}
