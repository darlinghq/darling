/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

 SPDX-License-Identifier: GPL-3.0-only
 SPDX-FileCopyrightText: 2026
*/

#include <CoreFoundation/CoreFoundation.h>
#include <HIToolbox/CarbonEvents.h>
#include "HIToolboxInternal.h"
#include <HIToolbox/Events.h>
#include <CoreGraphics/CoreGraphicsPrivate.h>
#include <CarbonCore/MacErrors.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

// Hot keys live in the window system of the host. HIToolbox sets them up through the
// window server SPI of CoreGraphics (CGSSetHotKeyWithExclusion, CGSRemoveHotKey), which the
// X11 backend implements by grabbing the key on the root window, so a hot key works
// wherever the keyboard focus is. The presses and releases come back as system-defined events
// in the application's event queue (subtype 6 and 9, data1 the uid of the hot key); NSApplication
// sends them, as Carbon raw events, to the event dispatcher target, whose standard handler (below)
// sends a kEventHotKeyPressed / kEventHotKeyReleased event to the target given at registration.
//
// Only one registration of a key code and modifiers combination can exist in a process.
// Unlike on macOS, where any number of processes can register the same hot key, a host
// window system hands a combination to a single client, so a combination another
// application holds is refused with eventHotKeyExistsErr.
struct OpaqueEventHotKeyRef
{
	struct OpaqueEventHotKeyRef* next;
	UInt32 keyCode;
	UInt32 modifiers;
	EventHotKeyID id;
	EventTargetRef target;
	OptionBits options;
	int uid;                // names the hot key in CoreGraphics
	CGSConnectionID connection;  // 0 if there is no window system
};

static pthread_mutex_t g_hotKeyLock = PTHREAD_MUTEX_INITIALIZER;
static struct OpaqueEventHotKeyRef* g_hotKeys;
static int g_nextHotKeyUID = 1;

// The standard handler of the raw system-defined event (on the event dispatcher target): the
// window system posts a hot key press or release as such an event, and AppKit sends it to the
// dispatcher target when the application receives it (on macOS too, the event comes through the
// application's event queue). Subtype 6 is a press, 9 a release, the first data word names the hot
// key by the uid HIToolbox gave CGSSetHotKeyWithExclusion (see CoreGraphicsPrivate.h for the
// evidence). The Carbon hot key event goes to the target of the registration.
__attribute__((visibility("hidden")))
OSStatus _HIStandardRawEventHandler(EventHandlerCallRef call, EventRef rawEvent, void* userData)
{
	CGEventRef cgEvent = CopyEventCGEvent(rawEvent);
	CGSEventRecord record;
	EventRef event = NULL;
	EventHotKeyID id = { 0, 0 };
	EventTargetRef target = NULL;
	OSStatus result = eventNotHandledErr;

	if (!cgEvent)
		return eventNotHandledErr;
	if (CGEventGetEventRecord(cgEvent, &record, sizeof(record)) != kCGErrorSuccess || record.type != NX_SYSDEFINED)
	{
		CFRelease(cgEvent);
		return eventNotHandledErr;
	}
	CFRelease(cgEvent);

	int subtype = record.data.compound.subType;
	int uid = record.data.compound.misc.L[0];
	if (subtype != kCGSHotKeyPressedSubtype && subtype != kCGSHotKeyReleasedSubtype)
		return eventNotHandledErr;

	// The registration may have been removed since the window system reported the key
	pthread_mutex_lock(&g_hotKeyLock);
	for (struct OpaqueEventHotKeyRef* hotKey = g_hotKeys; hotKey; hotKey = hotKey->next)
	{
		if (hotKey->uid == uid)
		{
			id = hotKey->id;
			target = hotKey->target;
			break;
		}
	}
	pthread_mutex_unlock(&g_hotKeyLock);
	if (!target)
		return eventNotHandledErr;

	if (CreateEvent(NULL, kEventClassKeyboard, subtype == kCGSHotKeyPressedSubtype ? kEventHotKeyPressed : kEventHotKeyReleased,
			0, kEventAttributeNone, &event) != noErr)
		return eventNotHandledErr;
	SetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, sizeof(id), &id);
	SendEventToEventTarget(event, target);
	result = noErr;
	ReleaseEvent(event);
	return result;
}

static CGSModifierFlags eventFlagsFromCarbonModifiers(UInt32 modifiers)
{
	CGEventFlags flags = 0;

	if (modifiers & cmdKey)
		flags |= kCGEventFlagMaskCommand;
	if (modifiers & shiftKey)
		flags |= kCGEventFlagMaskShift;
	if (modifiers & optionKey)
		flags |= kCGEventFlagMaskAlternate;
	if (modifiers & controlKey)
		flags |= kCGEventFlagMaskControl;
	return (CGSModifierFlags) flags;
}

static void unlinkHotKey(struct OpaqueEventHotKeyRef* hotKey)
{
	pthread_mutex_lock(&g_hotKeyLock);
	for (struct OpaqueEventHotKeyRef** link = &g_hotKeys; *link; link = &(*link)->next)
	{
		if (*link == hotKey)
		{
			*link = hotKey->next;
			break;
		}
	}
	pthread_mutex_unlock(&g_hotKeyLock);
}

OSStatus RegisterEventHotKey(UInt32 inHotKeyCode, UInt32 inHotKeyModifiers, EventHotKeyID inHotKeyID, EventTargetRef inTarget, OptionBits inOptions, EventHotKeyRef *outRef)
{
	if (!outRef || !inTarget || (inTarget != GetApplicationEventTarget() && inTarget != GetEventDispatcherTarget()))
		return paramErr;

	// QUIRK: the Caps Lock bit among the modifiers makes the registration fail with
	// eventInternalErr; no other modifier bit is refused, whether or not it is a
	// modifier a hot key can use. Not documented. Evidence: black-box observation on
	// macOS 26.7.3 (tier 2, mac-probes/v443-hotkey-modifier-bits.c).
	// Test: quirk_caps_lock_modifier_is_refused in tests/src/carbon_hotkeys.c
	if (inHotKeyModifiers & alphaLock)
		return eventInternalErr;

	struct OpaqueEventHotKeyRef* hotKey = calloc(1, sizeof(*hotKey));
	if (!hotKey)
		return memFullErr;
	hotKey->keyCode = inHotKeyCode;
	hotKey->modifiers = inHotKeyModifiers;
	hotKey->id = inHotKeyID;
	hotKey->target = inTarget;
	hotKey->options = inOptions;

	pthread_mutex_lock(&g_hotKeyLock);
	for (struct OpaqueEventHotKeyRef* other = g_hotKeys; other; other = other->next)
	{
		if (other->keyCode == inHotKeyCode && other->modifiers == inHotKeyModifiers)
		{
			pthread_mutex_unlock(&g_hotKeyLock);
			free(hotKey);
			return eventHotKeyExistsErr;
		}
	}

	// Listed before the host grabs the key, so that a press cannot arrive for a
	// registration that is not in the list yet
	hotKey->uid = g_nextHotKeyUID++;
	hotKey->next = g_hotKeys;
	g_hotKeys = hotKey;
	pthread_mutex_unlock(&g_hotKeyLock);

	// No window system (no display): the registration is kept, but no key press can ever arrive.
	hotKey->connection = CGSMainConnectionID();
	if (hotKey->connection != 0)
	{
		uint16_t key = 0;

		// The exclusion argument is not used. The virtual key code is what CoreGraphics takes as
		// the key (inference: the arguments of this SPI are not documented), options is unused.
		CGSSetHotKeyWithExclusion(hotKey->connection, hotKey->uid, 0, (uint16_t) inHotKeyCode,
			eventFlagsFromCarbonModifiers(inHotKeyModifiers), 0);

		// The setter does not return a status: a hot key the host did not accept (another
		// application holds the combination) is not stored, and is not found again.
		if (!CGSGetHotKey(hotKey->connection, hotKey->uid, NULL, &key, NULL))
		{
			unlinkHotKey(hotKey);
			free(hotKey);
			return eventHotKeyExistsErr;
		}
	}

	// QUIRK: a key code that names no key of the keyboard is accepted, and the hot key never
	// fires. Not documented. Evidence: black-box observation on macOS 26.7.3 (tier 2): code 200
	// is registered and unregistered without an error (mac-probes/v443-register-event-hotkey.c).
	// Test: quirk_unknown_key_code_registers in tests/src/carbon_hotkeys.c
	// (CoreGraphics keeps such a hot key without grabbing anything.)

	*outRef = hotKey;
	return noErr;
}

OSStatus UnregisterEventHotKey(EventHotKeyRef inHotKey)
{
	struct OpaqueEventHotKeyRef* hotKey = inHotKey;
	bool found = false;

	if (!hotKey)
		return paramErr;

	pthread_mutex_lock(&g_hotKeyLock);
	for (struct OpaqueEventHotKeyRef** link = &g_hotKeys; *link; link = &(*link)->next)
	{
		if (*link == hotKey)
		{
			*link = hotKey->next;
			found = true;
			break;
		}
	}
	pthread_mutex_unlock(&g_hotKeyLock);

	// Not (or no longer) registered
	if (!found)
		return paramErr;

	if (hotKey->connection != 0)
		CGSRemoveHotKey(hotKey->connection, hotKey->uid);
	free(hotKey);
	return noErr;
}

// Secure event input is a nesting count: every EnableSecureEventInput()
// needs a matching DisableSecureEventInput(). On macOS the window server
// combines the counts of all processes; Darling has no such server, so
// IsSecureEventInputEnabled() only reflects the calling process, and
// nothing restricts the delivery of keyboard events yet.
static int g_secureEventInputCount;

OSStatus EnableSecureEventInput(void)
{
	__atomic_add_fetch(&g_secureEventInputCount, 1, __ATOMIC_SEQ_CST);
	return noErr;
}

OSStatus DisableSecureEventInput(void)
{
	int count = __atomic_load_n(&g_secureEventInputCount, __ATOMIC_SEQ_CST);

	// An unbalanced call must not leave the count negative, which would
	// swallow the next EnableSecureEventInput().
	while (count > 0)
	{
		if (__atomic_compare_exchange_n(&g_secureEventInputCount, &count, count - 1,
				false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
			break;
	}
	return noErr;
}

Boolean IsSecureEventInputEnabled(void)
{
	return __atomic_load_n(&g_secureEventInputCount, __ATOMIC_SEQ_CST) > 0;
}

struct OpaqueEventLoopRef {
    CFRunLoopRef runLoop;
};

EventLoopRef GetMainEventLoop()
{
    static struct OpaqueEventLoopRef eventLoop;
    if (eventLoop.runLoop == NULL) {
        CFRunLoopRef runLoop = CFRunLoopGetMain();
        eventLoop.runLoop = runLoop;
    }
    return &eventLoop;
}

// TODO: use pthread stuff to hold the OpaqueEventLoopRef
//EventLoopRef GetCurrentEventLoop()
//{
//    return CFRunLoopGetCurrent();
//}

//void RunCurrentEventLoop(EventTimeout inTimeout)
//{
//    if (verbose) puts("Partial: RunCurrentEventLoop does not repsect the timeout.");
//    CFRunLoopRun();
//}

//OSStatus QuitEventLoop(EventLoopRef inEventLoop)
//{
//    EventLoopRef eventLoop = GetCurrentEventLoop();
//    CFRunLoopRef runLoop = (CFRunLoopRef) GetCFRunLoopFromEventLoop(eventLoop);
//    CFRunLoopStop(runLoop);
//    return noErr;
//}

CFTypeRef GetCFRunLoopFromEventLoop(EventLoopRef inEventLoop)
{
    return inEventLoop->runLoop;
}

void RunApplicationEventLoop(void)
{
    /*
    //if (verbose) puts("STUB: RunApplicationEventLoop called");
    puts("before CFRunLoopRun precheck");
    // TODO: one of these calls coredumps
    //CFRunLoopGetCurrent(); // Why does this segfault? I bet it just does this if there IS no runloop. There shouldn't be.
    puts("check to make sure the code is actually compiling");
    CFRunLoopRef main_rl = CFRunLoopGetMain();
    puts("before we GetCurrent");
    CFRunLoopRef current_rl = CFRunLoopGetCurrent(); // If I call this before CFRunLoopGetMain, it coredumps.
    puts("before comparison");
    if (current_rl != main_rl) {
        // TODO: what actually happens when it's wrongly called from a different thread?
        puts("RunApplicationEventLoop \"works only in the main event loop\" (quote from CarbonEvents.pdf)");
        exit(1);
        return;
    }
    */
    puts("literally before CFRunLoopRun");
    CFRunLoopRun();
    puts("after CFRunLoopRun");
    // TODO: does CFRunLoopRun install apple events, as per this URL?
    //       INFO: no, but this function can
    // https://leopard-adc.pepas.com/documentation/Carbon/Conceptual/Carbon_Event_Manager/CarbonEvents.pdf
    // TODO: per above URL this function also "installs a simple quit Apple event handler" (which is a fallback)
    // TODO: same URL above says it calls these:
    // ReceiveNextEvent(...)
    // GetEventDispatcherTarget(...)
    // SendEventToEventTarget(...)
    // ReleaseEvent(...)
    // TODO: also installs handlers for 
    //  - menu bar clicking
    //  - dispatching apple events (by calling AEProcessAppleEvent)
    //  - "respond to quit Apple events by quitting" the loop
    // TODO: "never explicity yields to other threads"
    // TODO: "Events sent to the standard toolbox dispatcher [the default
    // event target for this function] are automatically routed to the
    // appropriate event targets."
}

//void QuitApplicationEventLoop(void)
//{
//    EventLoopRef eventLoop = GetCurrentEventLoop();
//    CFRunLoopRef runLoop = (CFRunLoopRef) GetCFRunLoopFromEventLoop(eventLoop);
//    CFRunLoopStop(runLoop);
//}
