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

// The events of the window system: CoreGraphics keeps them, in the order they arrived and after the
// event taps have seen them, in the event stream of the process. The thread that waits for events
// in the main event loop reads the stream, and turns each CGEvent into an event of the main queue
// (CreateEventWithCGEvent). There is no thread of its own and no message per event: CoreGraphics
// signals a run loop source when the stream goes from empty to not empty, which ends the wait.
//
// Only HIToolbox depends on CoreGraphics here, never the other way round.

#include <HIToolbox/CarbonEventsCore.h>
#include <HIToolbox/CarbonEvents.h>
#include "HIToolboxInternal.h"
#include "HIToolboxPrivate.h"
#include <CoreGraphics/CoreGraphicsPrivate.h>
#include <AE/AEDataModel.h>

static atomic_bool g_streamRegistered;
static pthread_mutex_t g_streamRegisterLock = PTHREAD_MUTEX_INITIALIZER;
static CFRunLoopSourceRef g_streamSource;

static void streamSourceFired(void* info)
{
	// The signal only ends the wait; the events are read before the next look at the queue
}

// The main event loop becomes the reader of the stream: from now on the stream keeps the events,
// also between two waits
static void registerReader(struct OpaqueEventLoopRef* loop)
{
	if (atomic_load_explicit(&g_streamRegistered, memory_order_acquire))
		return;

	pthread_mutex_lock(&g_streamRegisterLock);
	if (!atomic_load(&g_streamRegistered))
	{
		CFRunLoopSourceContext context = { .version = 0, .perform = streamSourceFired };

		g_streamSource = CFRunLoopSourceCreate(NULL, 0, &context);
		CFRunLoopAddSource(loop->runLoop, g_streamSource, kCFRunLoopCommonModes);
		_CGSEventStreamSetReadySource(g_streamSource, loop->runLoop);
		atomic_store_explicit(&g_streamRegistered, true, memory_order_release);
	}
	pthread_mutex_unlock(&g_streamRegisterLock);
}

void _HIEventStreamWillWait(struct OpaqueEventLoopRef* loop, CFStringRef mode)
{
	// Window system events are for the main thread
	if (loop != _HIMainEventLoop() || !pthread_main_np())
		return;

	registerReader(loop);
	if (!CFEqual(mode, kCFRunLoopDefaultMode))
		CFRunLoopAddSource(loop->runLoop, g_streamSource, mode);
}

void _HIBeginEventStreamReading(void)
{
	registerReader(_HIMainEventLoop());
}

// The parameters of a raw key event of the window system, besides the key code and the modifiers that every
// event made from a CGEvent has: the characters the key produced (the Mac Roman character and the Unicode
// text) and the type of the keyboard. Their sources are the Unicode string and the keyboard type field of the
// CGEvent, which CoreGraphics fills from the keyboard layout in use.
static void addKeyParameters(EventRef event, CGEventRef cgEvent)
{
	UniChar text[16];
	UniCharCount length = 0;

	CGEventKeyboardGetUnicodeString(cgEvent, sizeof(text) / sizeof(text[0]), &length, text);
	if (length > 0)
	{
		UInt8 macChar = 0;
		CFStringRef string = CFStringCreateWithCharactersNoCopy(NULL, text, 1, kCFAllocatorNull);

		// A character the Mac Roman encoding has no place for stays 0
		if (string)
		{
			CFStringGetBytes(string, CFRangeMake(0, 1), kCFStringEncodingMacRoman, 0, false, &macChar, 1, NULL);
			CFRelease(string);
		}
		SetEventParameter(event, kEventParamKeyMacCharCodes, typeChar, sizeof(macChar), &macChar);
		SetEventParameter(event, kEventParamKeyUnicodes, typeUnicodeText, length * sizeof(UniChar), text);
	}

	UInt32 keyboardType = (UInt32) CGEventGetIntegerValueField(cgEvent, kCGKeyboardEventKeyboardType);

	SetEventParameter(event, kEventParamKeyboardType, typeUInt32, sizeof(keyboardType), &keyboardType);
}

static _Atomic(HIEventStreamFilterProc) g_streamFilter;

void _HISetEventStreamFilter(HIEventStreamFilterProc filter)
{
	atomic_store(&g_streamFilter, filter);
}

static void postStreamEvent(CGEventRef cgEvent, void* info)
{
	EventQueueRef queue = info;
	EventRef event = NULL;

	if (CreateEventWithCGEvent(NULL, cgEvent, kEventAttributeNone, &event) == noErr)
	{
		// CreateEventWithCGEvent() gives a key event the key code and the modifiers only (see
		// test_quirk_cgevent_classes); the raw key events of the event queue carry the documented
		// text and keyboard type as well.
		if (event->eventClass == kEventClassKeyboard && (event->kind == kEventRawKeyDown || event->kind == kEventRawKeyRepeat || event->kind == kEventRawKeyUp))
			addKeyParameters(event, cgEvent);
		_HIPostEventFromWaitingThread(queue, event);
		ReleaseEvent(event);
	}
}

void _HIDrainEventStream(EventQueueRef queue)
{
	if (!atomic_load_explicit(&g_streamRegistered, memory_order_acquire) || queue != &_HIMainEventLoop()->queue)
		return;

	// Until it is empty: the source is signalled again only after that
	CGEventRef cgEvent;
	while ((cgEvent = _CGSEventStreamCopyNextEvent()) != NULL)
	{
		HIEventStreamFilterProc filter = atomic_load(&g_streamFilter);

		if (filter != NULL)
			filter(cgEvent, postStreamEvent, queue);
		else
			postStreamEvent(cgEvent, queue);
		CFRelease(cgEvent);
	}
}
