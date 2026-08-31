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
*/

#ifndef _Carbon_Carbon_Events_H_
#define _Carbon_Carbon_Events_H_

#include <CoreFoundation/CFBase.h>
#include <CoreServices/MacTypes.h>
#include <HIToolbox/CarbonEventsCore.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EventHotKeyID {
    OSType signature;
    UInt32 id;
} EventHotKeyID;
typedef struct OpaqueEventHotKeyRef *EventHotKeyRef;

EventTargetRef GetApplicationEventTarget(void);
OSStatus GetEventDispatcherTarget();

OSStatus ProcessHICommand(const HICommand * a);

void RunApplicationEventLoop(void);

OSStatus RegisterEventHotKey(UInt32 inHotKeyCode, UInt32 inHotKeyModifiers, EventHotKeyID inHotKeyID, EventTargetRef inTarget, OptionBits inOptions, EventHotKeyRef *outRef);
OSStatus UnregisterEventHotKey(EventHotKeyRef inHotKey);

#ifdef __cplusplus
}
#endif

#endif
