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
#include "HIToolboxInternal.h"
#include <CoreGraphics/CGEvent.h>
#include <CoreGraphics/CGEventSource.h>

// The Carbon modifier bits (the bits above the low byte of EventModifiers) that correspond to the CGEventFlags
UInt32 _HIModifiersFromCGEventFlags(CGEventFlags flags)
{
    UInt32 modifiers = 0;

    if (flags & kCGEventFlagMaskCommand)
        modifiers |= 1 << 8;     // cmdKey
    if (flags & kCGEventFlagMaskShift)
        modifiers |= 1 << 9;     // shiftKey
    if (flags & kCGEventFlagMaskAlphaShift)
        modifiers |= 1 << 10;    // alphaLock
    if (flags & kCGEventFlagMaskAlternate)
        modifiers |= 1 << 11;    // optionKey
    if (flags & kCGEventFlagMaskControl)
        modifiers |= 1 << 12;    // controlKey
    return modifiers;
}

UInt32 GetCurrentKeyModifiers(void)
{
    return _HIModifiersFromCGEventFlags(CGEventSourceFlagsState(kCGEventSourceStateCombinedSessionState));
}

// The modifiers of the event being dispatched if it comes from a CGEvent, else the ones now
UInt32 GetCurrentEventKeyModifiers(void)
{
    EventRef current = GetCurrentEvent();
    CGEventRef cgEvent = current ? CopyEventCGEvent(current) : NULL;

    if (cgEvent)
    {
        UInt32 modifiers = _HIModifiersFromCGEventFlags(CGEventGetFlags(cgEvent));
        CFRelease(cgEvent);
        return modifiers;
    }
    return GetCurrentKeyModifiers();
}

// One bit for each mouse button that is down, the primary button in the lowest bit
UInt32 GetCurrentButtonState(void)
{
    UInt32 buttons = 0;

    for (int button = 0; button < 8; button++)
    {
        if (CGEventSourceButtonState(kCGEventSourceStateCombinedSessionState, (CGMouseButton) button))
            buttons |= 1u << button;
    }
    return buttons;
}

UInt32 GetCurrentEventButtonState(void)
{
    return GetCurrentButtonState();
}

EventTime GetCurrentEventTime(void)
{
    EventRef current = GetCurrentEvent();

    return current ? GetEventTime(current) : _HICurrentTime();
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
