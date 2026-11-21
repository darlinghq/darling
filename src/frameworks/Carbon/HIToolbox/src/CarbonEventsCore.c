/*
 This file is part of Darling.

 Copyright (C) 2025 Lubos Dolezel

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

#include <HIToolbox/CarbonEventsCore.h>

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

// Event queues are only identities here: nothing can be posted to one.
struct OpaqueEventQueueRef
{
    int unused;
};

static struct OpaqueEventQueueRef g_mainQueue;

EventQueueRef GetMainEventQueue(void)
{
    return &g_mainQueue;
}

// Every thread has an event queue of its own; the main thread's is the main queue.
EventQueueRef GetCurrentEventQueue(void)
{
    static _Thread_local struct OpaqueEventQueueRef threadQueue;

    return pthread_main_np() ? &g_mainQueue : &threadQueue;
}

// The event being dispatched by an event loop of the calling thread. No event loop dispatches
// events here, and an event sent with SendEventToEventTarget() is not reported either.
EventRef GetCurrentEvent(void)
{
    return NULL;
}

OSStatus PostEventToQueue(EventQueueRef a, EventRef b, EventPriority c)
{
    if (verbose) puts("STUB: PostEventToQueue called");
	return 0;
}

OSStatus ReceiveNextEvent(ItemCount a, const EventTypeSpec* b, EventTimeout c, Boolean d, EventRef* e)
{
    if (verbose) puts("STUB: ReceiveNextEvent called");
	return 0;
}

UInt32 GetCurrentKeyModifiers()
{
    if (verbose) puts("STUB: GetCurrentKeyModifiers called");
    return (UInt32) 0;
}

// Darling has no system symbolic hot keys, so no event ever matches one.
Boolean _IsSymbolicHotKeyEvent(EventRef inEvent, UInt32* outHotKeyCode, Boolean* outEnabled)
{
    if (outHotKeyCode)
        *outHotKeyCode = 0;
    if (outEnabled)
        *outEnabled = false;
    return false;
}
