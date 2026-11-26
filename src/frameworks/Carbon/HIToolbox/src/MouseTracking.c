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

// Mouse tracking loops of the Carbon Event Manager (32-bit only, like the QuickDraw ports they
// report in): TrackMouseLocation() and TrackMouseLocationWithOptions().

#ifndef __LP64__

#include <HIToolbox/CarbonEvents.h>
#include <QD/QD.h>
#include <CoreServices/CoreServices.h>
#include <CoreGraphics/CoreGraphics.h>

enum
{
	kHIKeyCodeEscape = 0x35,
	kHIKeyCodePeriod = 0x2F,
	kHICommandModifier = 1 << 8,
};

OSStatus TrackMouseLocationWithOptions(GrafPtr inPort, OptionBits inOptions, EventTimeout inTimeout, Point* outPt, UInt32* outModifiers, MouseTrackingResult* outResult)
{
	static const EventTypeSpec kTypes[] = {
		{ kEventClassMouse, kEventMouseDown }, { kEventClassMouse, kEventMouseUp },
		{ kEventClassMouse, kEventMouseMoved }, { kEventClassMouse, kEventMouseDragged },
		{ kEventClassKeyboard, kEventRawKeyDown }, { kEventClassKeyboard, kEventRawKeyModifiersChanged },
		{ kEventClassMouse, kEventMouseWheelMoved },
	};
	bool wantWheel = (inOptions & kTrackMouseLocationOptionIncludeScrollWheel) != 0;

	if (!outPt || !outResult)
		return paramErr;

	for (;;)
	{
		EventRef event = NULL;
		bool consume = true;
		OSStatus status = ReceiveNextEvent(GetEventTypeCount(kTypes) - (wantWheel ? 0 : 1), kTypes, inTimeout, false, &event);

		if (status == eventLoopTimedOutErr)
		{
			*outResult = kMouseTrackingTimedOut;
			// The location is where the mouse is now
			HIPoint where = { 0, 0 };
			CGEventRef current = CGEventCreate(NULL);
			if (current)
			{
				where = CGEventGetLocation(current);
				CFRelease(current);
			}
			outPt->h = (short) where.x;
			outPt->v = (short) where.y;
			if (inPort != (GrafPtr) -1)
			{
				GrafPtr port = inPort;
				if (!port)
					GetPort(&port);
				if (port)
					QDGlobalToLocalPoint(port, outPt);
			}
			if (outModifiers)
				*outModifiers = GetCurrentKeyModifiers();
			return noErr;
		}
		if (status != noErr)
			return status;

		UInt32 cls = GetEventClass(event);
		UInt32 kind = GetEventKind(event);
		MouseTrackingResult result = 0;
		HIPoint location = { 0, 0 };

		if (cls == kEventClassMouse)
		{
			switch (kind)
			{
				case kEventMouseDown: result = kMouseTrackingMouseDown; break;
				case kEventMouseUp:
					result = kMouseTrackingMouseUp;
					consume = (inOptions & kTrackMouseLocationOptionDontConsumeMouseUp) == 0;
					break;
				case kEventMouseDragged: result = kMouseTrackingMouseDragged; break;
				case kEventMouseMoved: result = kMouseTrackingMouseMoved; break;
				case kEventMouseWheelMoved: result = kMouseTrackingScrollWheel; break;
			}
			GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(location), NULL, &location);
		}
		else if (kind == kEventRawKeyModifiersChanged)
		{
			result = kMouseTrackingKeyModifiersChanged;
		}
		else if (kind == kEventRawKeyDown)
		{
			UInt32 keyCode = 0, modifiers = 0;

			GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyCode), NULL, &keyCode);
			GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modifiers), NULL, &modifiers);
			if (keyCode == kHIKeyCodeEscape || (keyCode == kHIKeyCodePeriod && (modifiers & kHICommandModifier)))
				result = kMouseTrackingUserCancelled;
			else
			{
				// Other keys are not for the tracking loop; they are dropped, so that they do not stay
				// in the queue and end every following wait at once
				RemoveEventFromQueue(GetCurrentEventQueue(), event);
				ReleaseEvent(event);
				continue;
			}
		}

		if (consume)
			RemoveEventFromQueue(GetCurrentEventQueue(), event);
		if (cls != kEventClassMouse)
		{
			CGEventRef current = CGEventCreate(NULL);
			if (current)
			{
				location = CGEventGetLocation(current);
				CFRelease(current);
			}
		}
		outPt->h = (short) location.x;
		outPt->v = (short) location.y;
		if (inPort != (GrafPtr) -1)
		{
			GrafPtr port = inPort;
			if (!port)
				GetPort(&port);
			if (port)
				QDGlobalToLocalPoint(port, outPt);
		}
		if (outModifiers)
			*outModifiers = GetCurrentKeyModifiers();
		*outResult = result;
		ReleaseEvent(event);
		return noErr;
	}
}

OSStatus TrackMouseLocation(GrafPtr inPort, Point* outPt, MouseTrackingResult* outResult)
{
	return TrackMouseLocationWithOptions(inPort, 0, kEventDurationForever, outPt, NULL, outResult);
}

#endif
