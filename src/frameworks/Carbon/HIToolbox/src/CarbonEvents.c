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
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

OSStatus RegisterEventHotKey(UInt32 inHotKeyCode, UInt32 inHotKeyModifiers, EventHotKeyID inHotKeyID, EventTargetRef inTarget, OptionBits inOptions, EventHotKeyRef *outRef)
{
    if (verbose) puts("STUB: RegisterEventHotKey called");

    return 0;
}

OSStatus UnregisterEventHotKey(EventHotKeyRef inHotKey)
{
    if (verbose) puts("STUB: UnregisterEventHotKey called");

    return 0;
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
