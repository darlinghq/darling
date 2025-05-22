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

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

OSStatus CallNextEventHandler(EventHandlerCallRef a, EventRef b)
{
    if (verbose) puts("STUB: CallNextEventHandler called");
	return 0;
}

OSStatus CreateEvent(CFAllocatorRef a, OSType b, UInt32 c, EventTime d, EventAttributes e, EventRef* f)
{
    if (verbose) puts("STUB: CreateEvent called");
	return 0;
}

OSType GetEventClass(EventRef a)
{
    if (verbose) puts("STUB: GetEventClass called");
	return 0;
}

UInt32 GetEventKind(EventRef a)
{
    if (verbose) puts("STUB: GetEventKind called");
	return 0;
}

OSStatus GetEventParameter(EventRef a, EventParamName b, EventParamType c, EventParamType* d, ByteCount e, ByteCount* f, void* g)
{
    if (verbose) puts("STUB: GetEventParameter called");
	return 0;
}

EventQueueRef GetMainEventQueue()
{
    if (verbose) puts("STUB: GetMainEventQueue called");
	return 0;
}

OSStatus InstallEventHandler(EventTargetRef a, EventHandlerUPP b, ItemCount c, const EventTypeSpec* d, void* e, EventHandlerRef* f)
{
    if (verbose) puts("STUB: InstallEventHandler called");
	return 0;
}

EventHandlerUPP NewEventHandlerUPP(EventHandlerProcPtr a)
{
    if (verbose) puts("STUB: NewEventHandlerUPP called");
	return (EventHandlerUPP)0;
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

void ReleaseEvent(EventRef a)
{
    if (verbose) puts("STUB: ReleaseEvent called");
}

OSStatus RemoveEventHandler(EventHandlerRef a)
{
    if (verbose) puts("STUB: RemoveEventHandler called");
	return 0;
}

OSStatus SendEventToEventTarget(EventRef a, EventTargetRef b)
{
    if (verbose) puts("STUB: SendEventToEventTarget called");
	return 0;
}
