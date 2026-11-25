/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
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

// A session's connection to diskarbitrationd, the disks it knows and its disk callbacks.
//
// The connection's handlers run on the session's own queue. They keep the disks the session knows up to date and
// queue a callout for every matching callback; the callouts run only on the run loops the session is scheduled on
// or on its dispatch queue. A scheduled session keeps itself alive until it is unscheduled, so a callout never
// outlives its session.
#include "DAPrivate.h"
#include "DAProtocol.h"

#include <DiskArbitration/DiskArbitrationPrivate.h>
#include <stdlib.h>
#include <string.h>

enum
{
	DA_CALLBACK_APPEARED,
	DA_CALLBACK_DISAPPEARED,
	DA_CALLBACK_CHANGED,
	DA_CALLBACK_UNMOUNT_APPROVAL,
	DA_CALLBACK_EJECT_APPROVAL,
	// not registered: a callout's own
	DA_CALLOUT_COMPLETION, // an unmount, eject or mount callback
	DA_CALLOUT_APPROVAL,   // an approval request from the daemon
};

struct DACallback
{
	int kind;
	void* function;
	void* context;
	CFDictionaryRef match; // NULL matches every disk
	CFArrayRef watch;      // for DA_CALLBACK_CHANGED: NULL watches every key
	bool removed;          // unregistered; its queued callouts are dropped
	int references;        // the registration and each queued callout
	struct DACallback* next;
};

struct DACallout
{
	struct DACallback* callback;
	char name[64];
	CFArrayRef keys;             // for DA_CALLBACK_CHANGED
	DADissenterRef dissenter;    // for DA_CALLOUT_COMPLETION; NULL for success
	xpc_object_t request;        // for DA_CALLOUT_APPROVAL: the daemon's request, answered after the callbacks
	CFDictionaryRef description; // for DA_CALLOUT_APPROVAL
	int approvalKind;            // for DA_CALLOUT_APPROVAL: DA_CALLBACK_UNMOUNT_APPROVAL or DA_CALLBACK_EJECT_APPROVAL
	struct DACallout* next;
};

// The connection's handlers outlive the session; they reach it through this, which says when it is gone. The
// handlers hold lock while they use the session; the references are atomic, so a handler can take one while it
// holds the lock.
struct DASessionLink
{
	pthread_mutex_t lock;
	DASessionRef session;        // NULL once the session is finalized
	xpc_connection_t connection; // the link's own reference, released once the connection is invalid, not before:
	                             // messages may still be arriving. The session holds another one until it is
	                             // finalized, so its requests never use a released connection.
	int references;
};

static struct DASessionLink* linkRetain(struct DASessionLink* link)
{
	__atomic_add_fetch(&link->references, 1, __ATOMIC_RELAXED);
	return link;
}

static void linkRelease(struct DASessionLink* link)
{
	if (__atomic_sub_fetch(&link->references, 1, __ATOMIC_ACQ_REL) == 0)
	{
		pthread_mutex_destroy(&link->lock);
		free(link);
	}
}

static bool matches(CFDictionaryRef match, CFDictionaryRef description)
{
	if (match == NULL)
		return true;
	if (description == NULL)
		return false;

	CFIndex count = CFDictionaryGetCount(match);
	const void** keys = malloc(sizeof(void*) * (count + 1));
	const void** values = malloc(sizeof(void*) * (count + 1));
	bool result = keys != NULL && values != NULL;

	if (result)
	{
		CFDictionaryGetKeysAndValues(match, keys, values);
		for (CFIndex i = 0; i < count && result; i++)
		{
			CFTypeRef value = CFDictionaryGetValue(description, keys[i]);
			result = value != NULL && CFEqual(value, values[i]);
		}
	}
	free(keys);
	free(values);
	return result;
}

static bool watched(CFArrayRef watch, CFArrayRef keys)
{
	if (watch == NULL)
		return true;
	for (CFIndex i = 0; keys != NULL && i < CFArrayGetCount(keys); i++)
	{
		if (CFArrayContainsValue(watch, CFRangeMake(0, CFArrayGetCount(watch)), CFArrayGetValueAtIndex(keys, i)))
			return true;
	}
	return false;
}

static void callbackReleaseLocked(struct DACallback* callback)
{
	if (--callback->references > 0)
		return;
	if (callback->match != NULL)
		CFRelease(callback->match);
	if (callback->watch != NULL)
		CFRelease(callback->watch);
	free(callback);
}

static void appendCalloutLocked(DASessionRef session, struct DACallout* callout)
{
	*session->calloutsTail = callout;
	session->calloutsTail = &callout->next;
}

static void queueCalloutLocked(DASessionRef session, struct DACallback* callback, CFStringRef name,
	CFDictionaryRef description, CFArrayRef keys)
{
	if (callback->removed || !matches(callback->match, description))
		return;
	if (callback->kind == DA_CALLBACK_CHANGED && !watched(callback->watch, keys))
		return;

	struct DACallout* callout = calloc(1, sizeof(*callout));
	if (callout == NULL)
		return;
	if (!CFStringGetCString(name, callout->name, sizeof(callout->name), kCFStringEncodingUTF8))
	{
		free(callout);
		return;
	}
	callout->callback = callback;
	callback->references++;
	if (keys != NULL)
		callout->keys = CFRetain(keys);
	appendCalloutLocked(session, callout);
}

static void queueLocked(DASessionRef session, int kind, CFStringRef name, CFDictionaryRef description, CFArrayRef keys)
{
	for (struct DACallback* callback = session->callbacks; callback != NULL; callback = callback->next)
	{
		if (callback->kind == kind)
			queueCalloutLocked(session, callback, name, description, keys);
	}
}

static void calloutFree(struct DACallout* callout)
{
	if (callout->keys != NULL)
		CFRelease(callout->keys);
	if (callout->dissenter != NULL)
		CFRelease(callout->dissenter);
	if (callout->request != NULL)
		xpc_release(callout->request);
	if (callout->description != NULL)
		CFRelease(callout->description);
	free(callout);
}

// A callout with a callback of its own, which no registration holds.
static struct DACallout* newPrivateCallout(int kind, const char* name, void* function, void* context)
{
	struct DACallout* callout = calloc(1, sizeof(*callout));
	struct DACallback* callback = calloc(1, sizeof(*callback));
	if (callout == NULL || callback == NULL || strlcpy(callout->name, name, sizeof(callout->name)) >= sizeof(callout->name))
	{
		free(callout);
		free(callback);
		return NULL;
	}
	callback->kind = kind;
	callback->function = function;
	callback->context = context;
	callback->references = 1;
	callout->callback = callback;
	return callout;
}

static void replyToApproval(DASessionRef session, xpc_object_t request, DADissenterRef dissenter)
{
	xpc_object_t reply = xpc_dictionary_create_reply(request);
	if (reply == NULL)
		return;
	xpc_dictionary_set_int64(reply, DA_KEY_STATUS, dissenter != NULL ? DADissenterGetStatus(dissenter) : kDAReturnSuccess);
	CFStringRef string = dissenter != NULL ? DADissenterGetStatusString(dissenter) : NULL;
	char text[1024];
	if (string != NULL && CFStringGetCString(string, text, sizeof(text), kCFStringEncodingUTF8))
		xpc_dictionary_set_string(reply, DA_KEY_STRING, text);
	xpc_connection_send_message(session->connection, reply);
	xpc_release(reply);
}

// Runs the matching approval callbacks until one dissents, and answers the daemon.
static void runApproval(DASessionRef session, DADiskRef disk, struct DACallout* callout)
{
	struct DACallback* callbacks[64];
	int count = 0;

	pthread_mutex_lock(&session->lock);
	for (struct DACallback* callback = session->callbacks; callback != NULL && count < 64; callback = callback->next)
	{
		if (callback->kind == callout->approvalKind && !callback->removed
			&& matches(callback->match, callout->description))
		{
			callback->references++;
			callbacks[count++] = callback;
		}
	}
	pthread_mutex_unlock(&session->lock);

	DADissenterRef dissenter = NULL;
	for (int i = 0; i < count; i++)
	{
		pthread_mutex_lock(&session->lock);
		bool removed = callbacks[i]->removed;
		pthread_mutex_unlock(&session->lock);
		if (dissenter == NULL && !removed)
			dissenter = ((DADiskUnmountApprovalCallback) callbacks[i]->function)(disk, callbacks[i]->context);
	}

	pthread_mutex_lock(&session->lock);
	for (int i = 0; i < count; i++)
		callbackReleaseLocked(callbacks[i]);
	pthread_mutex_unlock(&session->lock);

	replyToApproval(session, callout->request, dissenter);
	if (dissenter != NULL)
		CFRelease(dissenter); // the callback's dissenter belongs to the framework
}

// Runs the queued callouts; on a run loop the session is scheduled on, or on its dispatch queue.
static void deliver(DASessionRef session)
{
	CFRetain(session); // a callout may unschedule and release the session

	pthread_mutex_lock(&session->lock);
	struct DACallout* callout = session->callouts;
	session->callouts = NULL;
	session->calloutsTail = &session->callouts;
	pthread_mutex_unlock(&session->lock);

	while (callout != NULL)
	{
		struct DACallout* next = callout->next;
		struct DACallback* callback = callout->callback;

		pthread_mutex_lock(&session->lock);
		bool removed = callback->removed;
		pthread_mutex_unlock(&session->lock);

		DADiskRef disk = DADiskCreateFromBSDName(NULL, session, callout->name);
		if (disk != NULL && callback->kind == DA_CALLOUT_APPROVAL)
		{
			runApproval(session, disk, callout);
		}
		else if (disk != NULL && !removed)
		{
			switch (callback->kind)
			{
				case DA_CALLBACK_APPEARED:
					((DADiskAppearedCallback) callback->function)(disk, callback->context);
					break;
				case DA_CALLBACK_DISAPPEARED:
					((DADiskDisappearedCallback) callback->function)(disk, callback->context);
					break;
				case DA_CALLBACK_CHANGED:
					((DADiskDescriptionChangedCallback) callback->function)(disk, callout->keys, callback->context);
					break;
				case DA_CALLOUT_COMPLETION:
					((DADiskUnmountCallback) callback->function)(disk, callout->dissenter, callback->context);
					break;
			}
		}
		if (disk != NULL)
			CFRelease(disk);

		pthread_mutex_lock(&session->lock);
		callbackReleaseLocked(callback);
		pthread_mutex_unlock(&session->lock);
		calloutFree(callout);
		callout = next;
	}

	CFRelease(session);
}

static void performSource(void* info)
{
	deliver((DASessionRef) info);
}

// Has the queued callouts delivered where the session is scheduled; an unscheduled session keeps them.
static void signalLocked(DASessionRef session)
{
	if (session->callouts == NULL)
		return;

	if (session->source != NULL && CFArrayGetCount(session->schedules) > 0)
	{
		CFRunLoopSourceSignal(session->source);
		for (CFIndex i = 0; i < CFArrayGetCount(session->schedules); i++)
		{
			CFArrayRef schedule = CFArrayGetValueAtIndex(session->schedules, i);
			CFRunLoopWakeUp((CFRunLoopRef) CFArrayGetValueAtIndex(schedule, 0));
		}
	}
	if (session->target != NULL)
	{
		CFRetain(session);
		dispatch_async(session->target, ^{
			deliver(session);
			CFRelease(session);
		});
	}
}

// Keeps a scheduled session alive. True if the caller must release the session once it has unlocked.
static bool updateSelfRetainLocked(DASessionRef session)
{
	bool scheduled = session->target != NULL || CFArrayGetCount(session->schedules) > 0;

	if (scheduled && !session->selfRetained)
	{
		CFRetain(session);
		session->selfRetained = true;
	}
	else if (!scheduled && session->selfRetained)
	{
		session->selfRetained = false;
		return true;
	}
	return false;
}

static void knowLocked(DASessionRef session, CFStringRef name, CFDictionaryRef description)
{
	if (!CFDictionaryContainsKey(session->disks, name))
		CFArrayAppendValue(session->diskNames, name);
	CFDictionarySetValue(session->disks, name, description);
}

static void forgetLocked(DASessionRef session, CFStringRef name)
{
	CFIndex index = CFArrayGetFirstIndexOfValue(session->diskNames, CFRangeMake(0, CFArrayGetCount(session->diskNames)),
		name);
	if (index >= 0)
		CFArrayRemoveValueAtIndex(session->diskNames, index);
	CFDictionaryRemoveValue(session->disks, name);
}

static void appearedLocked(DASessionRef session, CFStringRef name, CFDictionaryRef description)
{
	if (CFDictionaryContainsKey(session->disks, name))
	{
		knowLocked(session, name, description);
		return;
	}
	knowLocked(session, name, description);
	queueLocked(session, DA_CALLBACK_APPEARED, name, description, NULL);
}

static void disappearedLocked(DASessionRef session, CFStringRef name)
{
	CFDictionaryRef last = CFDictionaryGetValue(session->disks, name);
	if (last == NULL)
		return;
	CFRetain(last);
	forgetLocked(session, name);
	queueLocked(session, DA_CALLBACK_DISAPPEARED, name, last, NULL);
	CFRelease(last);
}

static CFArrayRef keysFromMessage(xpc_object_t message)
{
	xpc_object_t array = xpc_dictionary_get_value(message, DA_KEY_KEYS);
	CFMutableArrayRef keys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	if (array != NULL && xpc_get_type(array) == XPC_TYPE_ARRAY)
	{
		for (size_t i = 0; i < xpc_array_get_count(array); i++)
		{
			const char* key = xpc_array_get_string(array, i);
			if (key == NULL)
				continue;
			CFStringRef string = CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
			if (string != NULL)
			{
				CFArrayAppendValue(keys, string);
				CFRelease(string);
			}
		}
	}
	return keys;
}

// An event message from the daemon. On the session's queue.
static void applyEvent(DASessionRef session, xpc_object_t event)
{
	const char* kind = xpc_dictionary_get_string(event, DA_KEY_EVENT);
	const char* name = xpc_dictionary_get_string(event, DA_KEY_NAME);
	if (kind == NULL || name == NULL)
		return;

	CFStringRef cfName = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
	CFMutableDictionaryRef description = _DAClientDescriptionFromMessage(event);
	if (cfName == NULL || description == NULL)
		goto out;

	pthread_mutex_lock(&session->lock);
	if (strcmp(kind, DA_EVENT_APPEARED) == 0)
	{
		appearedLocked(session, cfName, description);
	}
	else if (strcmp(kind, DA_EVENT_DISAPPEARED) == 0)
	{
		disappearedLocked(session, cfName);
	}
	else if (strcmp(kind, DA_EVENT_CHANGED) == 0 && CFDictionaryContainsKey(session->disks, cfName))
	{
		CFArrayRef keys = keysFromMessage(event);
		knowLocked(session, cfName, description);
		queueLocked(session, DA_CALLBACK_CHANGED, cfName, description, keys);
		CFRelease(keys);
	}
	signalLocked(session);
	pthread_mutex_unlock(&session->lock);

out:
	if (cfName != NULL)
		CFRelease(cfName);
	if (description != NULL)
		CFRelease(description);
}

// The keys whose values differ between two descriptions of a disk, not counting its appearance time.
static CFArrayRef changedKeys(CFDictionaryRef before, CFDictionaryRef after)
{
	CFMutableArrayRef keys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	CFDictionaryRef both[] = { before, after };

	for (int d = 0; d < 2; d++)
	{
		CFIndex count = CFDictionaryGetCount(both[d]);
		const void** names = malloc(sizeof(void*) * (count + 1));
		if (names == NULL)
			continue;
		CFDictionaryGetKeysAndValues(both[d], names, NULL);
		for (CFIndex i = 0; i < count; i++)
		{
			if (CFEqual(names[i], kDADiskDescriptionAppearanceTimeKey)
				|| CFArrayContainsValue(keys, CFRangeMake(0, CFArrayGetCount(keys)), names[i]))
				continue;
			CFTypeRef a = CFDictionaryGetValue(before, names[i]);
			CFTypeRef b = CFDictionaryGetValue(after, names[i]);
			if (a == NULL || b == NULL || !CFEqual(a, b))
				CFArrayAppendValue(keys, names[i]);
		}
		free(names);
	}
	return keys;
}

static bool isWholeName(CFStringRef name)
{
	char string[64];
	return CFStringGetCString(name, string, sizeof(string), kCFStringEncodingUTF8) && strchr(string + 4, 's') == NULL;
}

// The reply to hello lists the daemon's disks: the session catches up with it, then with the events that arrived
// before it. On the session's queue.
static void reconcile(DASessionRef session, xpc_object_t reply)
{
	xpc_object_t disks = xpc_dictionary_get_value(reply, DA_KEY_DISKS);
	CFMutableArrayRef names = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	CFMutableDictionaryRef descriptions = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);

	for (size_t i = 0; disks != NULL && xpc_get_type(disks) == XPC_TYPE_ARRAY && i < xpc_array_get_count(disks); i++)
	{
		xpc_object_t disk = xpc_array_get_value(disks, i);
		const char* name = xpc_get_type(disk) == XPC_TYPE_DICTIONARY ? xpc_dictionary_get_string(disk, DA_KEY_NAME)
			: NULL;
		CFMutableDictionaryRef description = name != NULL ? _DAClientDescriptionFromMessage(disk) : NULL;
		if (description == NULL)
			continue;
		CFStringRef cfName = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
		if (cfName != NULL)
		{
			CFArrayAppendValue(names, cfName);
			CFDictionarySetValue(descriptions, cfName, description);
			CFRelease(cfName);
		}
		CFRelease(description);
	}

	pthread_mutex_lock(&session->lock);

	// the disks that went away while the session was not connected: whole disks first, then leaves
	for (int pass = 0; pass < 2; pass++)
	{
		CFArrayRef known = CFArrayCreateCopy(NULL, session->diskNames);
		for (CFIndex i = 0; i < CFArrayGetCount(known); i++)
		{
			CFStringRef name = CFArrayGetValueAtIndex(known, i);
			if (isWholeName(name) == (pass == 0) && !CFDictionaryContainsKey(descriptions, name))
				disappearedLocked(session, name);
		}
		CFRelease(known);
	}

	for (CFIndex i = 0; i < CFArrayGetCount(names); i++)
	{
		CFStringRef name = CFArrayGetValueAtIndex(names, i);
		CFDictionaryRef description = CFDictionaryGetValue(descriptions, name);
		CFDictionaryRef before = CFDictionaryGetValue(session->disks, name);
		if (before == NULL)
		{
			appearedLocked(session, name, description);
			continue;
		}
		CFArrayRef keys = changedKeys(before, description);
		knowLocked(session, name, description);
		if (CFArrayGetCount(keys) > 0)
			queueLocked(session, DA_CALLBACK_CHANGED, name, description, keys);
		CFRelease(keys);
	}

	signalLocked(session);
	pthread_mutex_unlock(&session->lock);
	CFRelease(names);
	CFRelease(descriptions);

	xpc_object_t early = session->earlyEvents;
	session->earlyEvents = NULL;
	session->helloPending = false;
	for (size_t i = 0; early != NULL && i < xpc_array_get_count(early); i++)
		applyEvent(session, xpc_array_get_value(early, i));
	if (early != NULL)
		xpc_release(early);
}

// Subscribes to the daemon's events. On the session's queue; again after the connection was interrupted (the daemon
// restarted).
static void sayHello(DASessionRef session)
{
	struct DASessionLink* link = linkRetain(session->link);
	unsigned number = ++session->helloNumber;

	session->helloPending = true;
	if (session->earlyEvents != NULL)
		xpc_release(session->earlyEvents);
	session->earlyEvents = xpc_array_create(NULL, 0);

	xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(message, DA_KEY_OP, DA_OP_HELLO);
	xpc_connection_send_message_with_reply(session->connection, message, session->queue, ^(xpc_object_t reply) {
		pthread_mutex_lock(&link->lock);
		DASessionRef current = link->session;
		if (current != NULL && current->helloNumber == number && xpc_get_type(reply) == XPC_TYPE_DICTIONARY)
			reconcile(current, reply);
		pthread_mutex_unlock(&link->lock);
		linkRelease(link);
	});
	xpc_release(message);
}

static bool isScheduledLocked(DASessionRef session)
{
	return session->target != NULL || CFArrayGetCount(session->schedules) > 0;
}

// An approval request: it waits for the session's matching approval callbacks, if the session has any and is
// scheduled; otherwise it is approved at once. On the session's queue.
static void handleApproval(DASessionRef session, xpc_object_t request)
{
	const char* kind = xpc_dictionary_get_string(request, DA_KEY_KIND);
	const char* name = xpc_dictionary_get_string(request, DA_KEY_NAME);
	int approvalKind = kind == NULL ? -1 : strcmp(kind, DA_OP_UNMOUNT) == 0 ? DA_CALLBACK_UNMOUNT_APPROVAL
		: strcmp(kind, DA_OP_EJECT) == 0 ? DA_CALLBACK_EJECT_APPROVAL : -1;
	CFMutableDictionaryRef description = _DAClientDescriptionFromMessage(request);
	bool queued = false;

	pthread_mutex_lock(&session->lock);
	bool wanted = false;
	for (struct DACallback* callback = session->callbacks; callback != NULL && !wanted; callback = callback->next)
	{
		wanted = callback->kind == approvalKind && !callback->removed && description != NULL
			&& matches(callback->match, description);
	}
	if (wanted && name != NULL && isScheduledLocked(session))
	{
		struct DACallout* callout = newPrivateCallout(DA_CALLOUT_APPROVAL, name, NULL, NULL);
		if (callout != NULL)
		{
			callout->request = xpc_retain(request);
			callout->description = CFRetain(description);
			callout->approvalKind = approvalKind;
			appendCalloutLocked(session, callout);
			signalLocked(session);
			queued = true;
		}
	}
	pthread_mutex_unlock(&session->lock);

	if (!queued)
		replyToApproval(session, request, NULL);
	if (description != NULL)
		CFRelease(description);
}

static bool isApproval(xpc_object_t event)
{
	const char* kind = xpc_dictionary_get_string(event, DA_KEY_EVENT);
	return kind != NULL && strcmp(kind, DA_EVENT_APPROVE) == 0;
}

static void handleEvent(struct DASessionLink* link, xpc_object_t event)
{
	pthread_mutex_lock(&link->lock);
	DASessionRef session = link->session;
	if (session != NULL)
	{
		if (event == XPC_ERROR_CONNECTION_INTERRUPTED)
			sayHello(session);
		else if (xpc_get_type(event) == XPC_TYPE_DICTIONARY && isApproval(event))
			handleApproval(session, event);
		else if (xpc_get_type(event) == XPC_TYPE_DICTIONARY && session->helloPending)
			xpc_array_append_value(session->earlyEvents, event);
		else if (xpc_get_type(event) == XPC_TYPE_DICTIONARY)
			applyEvent(session, event);
	}
	pthread_mutex_unlock(&link->lock);

	if (event == XPC_ERROR_CONNECTION_INVALID)
	{
		xpc_release(link->connection);
		linkRelease(link);
	}
}

// Connects to diskarbitrationd. Without the daemon the session still works; it then knows no disks.
void _DASessionConnect(DASessionRef session)
{
	pthread_mutex_init(&session->lock, NULL);
	session->diskNames = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	session->disks = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	session->schedules = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	session->calloutsTail = &session->callouts;

	struct DASessionLink* link = calloc(1, sizeof(*link));
	if (link == NULL)
		return;
	pthread_mutex_init(&link->lock, NULL);
	link->session = session;
	link->references = 1;
	session->link = link;

	session->queue = dispatch_queue_create("com.apple.DiskArbitration.session", NULL);
	session->connection = xpc_connection_create_mach_service(DA_SERVICE_NAME, session->queue, 0);
	if (session->connection == NULL)
		return;

	link->connection = xpc_retain(session->connection);
	linkRetain(link); // for the connection, until it is invalid
	xpc_connection_set_event_handler(session->connection, ^(xpc_object_t event) {
		handleEvent(link, event);
	});
	xpc_connection_resume(session->connection);
	dispatch_sync(session->queue, ^{
		sayHello(session);
	});
}

void _DASessionDisconnect(DASessionRef session)
{
	if (session->link != NULL)
	{
		pthread_mutex_lock(&session->link->lock);
		session->link->session = NULL;
		pthread_mutex_unlock(&session->link->lock);
		linkRelease(session->link);
	}
	// the connection's handler releases the link's reference once it is invalid
	if (session->connection != NULL)
	{
		xpc_connection_cancel(session->connection);
		xpc_release(session->connection);
		session->connection = NULL;
	}
	if (session->queue != NULL)
		dispatch_release(session->queue);
	if (session->earlyEvents != NULL)
		xpc_release(session->earlyEvents);

	while (session->callouts != NULL)
	{
		struct DACallout* callout = session->callouts;
		session->callouts = callout->next;
		callbackReleaseLocked(callout->callback);
		calloutFree(callout);
	}
	while (session->callbacks != NULL)
	{
		struct DACallback* callback = session->callbacks;
		session->callbacks = callback->next;
		callbackReleaseLocked(callback);
	}
	if (session->source != NULL)
	{
		CFRunLoopSourceInvalidate(session->source);
		CFRelease(session->source);
	}
	if (session->schedules != NULL)
		CFRelease(session->schedules);
	if (session->diskNames != NULL)
		CFRelease(session->diskNames);
	if (session->disks != NULL)
		CFRelease(session->disks);
	pthread_mutex_destroy(&session->lock);
}

void DASessionScheduleWithRunLoop(DASessionRef session, CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
	if (session == NULL || runLoop == NULL || runLoopMode == NULL)
		return;

	pthread_mutex_lock(&session->lock);
	if (session->source == NULL)
	{
		CFRunLoopSourceContext context = { .version = 0, .info = session, .perform = performSource };
		session->source = CFRunLoopSourceCreate(NULL, 0, &context);
	}

	const void* pair[] = { runLoop, runLoopMode };
	CFArrayRef schedule = CFArrayCreate(NULL, pair, 2, &kCFTypeArrayCallBacks);
	if (!CFArrayContainsValue(session->schedules, CFRangeMake(0, CFArrayGetCount(session->schedules)), schedule))
	{
		CFArrayAppendValue(session->schedules, schedule);
		CFRunLoopAddSource(runLoop, session->source, runLoopMode);
	}
	CFRelease(schedule);

	updateSelfRetainLocked(session);
	signalLocked(session);
	pthread_mutex_unlock(&session->lock);
}

void DASessionUnscheduleFromRunLoop(DASessionRef session, CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
	if (session == NULL || runLoop == NULL || runLoopMode == NULL)
		return;

	pthread_mutex_lock(&session->lock);
	const void* pair[] = { runLoop, runLoopMode };
	CFArrayRef schedule = CFArrayCreate(NULL, pair, 2, &kCFTypeArrayCallBacks);
	CFIndex index = CFArrayGetFirstIndexOfValue(session->schedules,
		CFRangeMake(0, CFArrayGetCount(session->schedules)), schedule);
	if (index >= 0)
	{
		CFRunLoopRemoveSource(runLoop, session->source, runLoopMode);
		CFArrayRemoveValueAtIndex(session->schedules, index);
	}
	CFRelease(schedule);
	bool release = updateSelfRetainLocked(session);
	pthread_mutex_unlock(&session->lock);

	if (release)
		CFRelease(session);
}

void DASessionSetDispatchQueue(DASessionRef session, dispatch_queue_t queue)
{
	if (session == NULL)
		return;

	if (queue != NULL)
		dispatch_retain(queue);
	pthread_mutex_lock(&session->lock);
	dispatch_queue_t old = session->target;
	session->target = queue;
	bool release = updateSelfRetainLocked(session);
	signalLocked(session);
	pthread_mutex_unlock(&session->lock);

	if (old != NULL)
		dispatch_release(old);
	if (release)
		CFRelease(session);
}

static void registerCallback(DASessionRef session, int kind, void* function, void* context, CFDictionaryRef match,
	CFArrayRef watch)
{
	if (session == NULL || function == NULL)
		return;

	struct DACallback* callback = calloc(1, sizeof(*callback));
	if (callback == NULL)
		return;
	callback->kind = kind;
	callback->function = function;
	callback->context = context;
	callback->match = match != NULL ? CFDictionaryCreateCopy(NULL, match) : NULL;
	callback->watch = watch != NULL ? CFArrayCreateCopy(NULL, watch) : NULL;
	callback->references = 1;

	pthread_mutex_lock(&session->lock);
	struct DACallback** tail = &session->callbacks;
	while (*tail != NULL)
		tail = &(*tail)->next;
	*tail = callback;

	// an appeared callback hears about the disks there already are
	if (kind == DA_CALLBACK_APPEARED)
	{
		for (CFIndex i = 0; i < CFArrayGetCount(session->diskNames); i++)
		{
			CFStringRef name = CFArrayGetValueAtIndex(session->diskNames, i);
			queueCalloutLocked(session, callback, name, CFDictionaryGetValue(session->disks, name), NULL);
		}
		signalLocked(session);
	}
	pthread_mutex_unlock(&session->lock);
}

void DARegisterDiskAppearedCallback(DASessionRef session, CFDictionaryRef match, DADiskAppearedCallback callback,
	void* context)
{
	registerCallback(session, DA_CALLBACK_APPEARED, callback, context, match, NULL);
}

void DARegisterDiskDisappearedCallback(DASessionRef session, CFDictionaryRef match, DADiskDisappearedCallback callback,
	void* context)
{
	registerCallback(session, DA_CALLBACK_DISAPPEARED, callback, context, match, NULL);
}

void DARegisterDiskDescriptionChangedCallback(DASessionRef session, CFDictionaryRef match, CFArrayRef watch,
	DADiskDescriptionChangedCallback callback, void* context)
{
	registerCallback(session, DA_CALLBACK_CHANGED, callback, context, match, watch);
}

void DAUnregisterCallback(DASessionRef session, void* callback, void* context)
{
	if (session == NULL)
		return;

	pthread_mutex_lock(&session->lock);
	for (struct DACallback** link = &session->callbacks; *link != NULL;)
	{
		struct DACallback* registered = *link;
		if (registered->function == callback && registered->context == context && registered->kind <= DA_CALLBACK_CHANGED)
		{
			*link = registered->next;
			registered->removed = true;
			callbackReleaseLocked(registered);
		}
		else
		{
			link = &registered->next;
		}
	}
	pthread_mutex_unlock(&session->lock);
}

void DARegisterDiskUnmountApprovalCallback(DASessionRef session, CFDictionaryRef match,
	DADiskUnmountApprovalCallback callback, void* context)
{
	registerCallback(session, DA_CALLBACK_UNMOUNT_APPROVAL, callback, context, match, NULL);
}

void DARegisterDiskEjectApprovalCallback(DASessionRef session, CFDictionaryRef match,
	DADiskEjectApprovalCallback callback, void* context)
{
	registerCallback(session, DA_CALLBACK_EJECT_APPROVAL, callback, context, match, NULL);
}

void DAUnregisterApprovalCallback(DASessionRef session, void* callback, void* context)
{
	if (session == NULL)
		return;

	pthread_mutex_lock(&session->lock);
	for (struct DACallback** link = &session->callbacks; *link != NULL;)
	{
		struct DACallback* registered = *link;
		bool approval = registered->kind == DA_CALLBACK_UNMOUNT_APPROVAL || registered->kind == DA_CALLBACK_EJECT_APPROVAL;
		if (approval && registered->function == callback && registered->context == context)
		{
			*link = registered->next;
			registered->removed = true;
			callbackReleaseLocked(registered);
		}
		else
		{
			link = &registered->next;
		}
	}
	pthread_mutex_unlock(&session->lock);
}

// Queues a request's completion callback, run where the session is scheduled.
void _DASessionComplete(DASessionRef session, const char* name, void* function, void* context, DAReturn status,
	CFStringRef string)
{
	if (function == NULL)
		return;
	struct DACallout* callout = newPrivateCallout(DA_CALLOUT_COMPLETION, name, function, context);
	if (callout == NULL)
		return;
	if (status != kDAReturnSuccess)
		callout->dissenter = DADissenterCreate(NULL, status, string);

	pthread_mutex_lock(&session->lock);
	appendCalloutLocked(session, callout);
	signalLocked(session);
	pthread_mutex_unlock(&session->lock);
}

void _DASessionRequest(DASessionRef session, const char* op, const char* name, uint64_t options, void* function,
	void* context)
{
	if (session->connection == NULL)
	{
		_DASessionComplete(session, name, function, context, kDAReturnNotReady, NULL);
		return;
	}

	xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(message, DA_KEY_OP, op);
	xpc_dictionary_set_string(message, DA_KEY_NAME, name);
	xpc_dictionary_set_uint64(message, DA_KEY_OPTIONS, options);

	struct DASessionLink* link = linkRetain(session->link);
	char* disk = strdup(name);
	xpc_connection_send_message_with_reply(session->connection, message, session->queue, ^(xpc_object_t reply) {
		pthread_mutex_lock(&link->lock);
		DASessionRef current = link->session;
		if (current != NULL && disk != NULL)
		{
			DAReturn status = kDAReturnError; // the daemon went away before it answered
			CFStringRef string = NULL;
			if (xpc_get_type(reply) == XPC_TYPE_DICTIONARY)
			{
				status = (DAReturn) xpc_dictionary_get_int64(reply, DA_KEY_STATUS);
				const char* text = xpc_dictionary_get_string(reply, DA_KEY_STRING);
				if (text != NULL)
					string = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
			}
			_DASessionComplete(current, disk, function, context, status, string);
			if (string != NULL)
				CFRelease(string);
		}
		pthread_mutex_unlock(&link->lock);
		linkRelease(link);
		free(disk);
	});
	xpc_release(message);
}
