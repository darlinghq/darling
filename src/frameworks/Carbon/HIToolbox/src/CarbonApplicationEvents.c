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

// The application events about the front process. The front process is state of the window system
// layer: CoreGraphics keeps it, and calls back when it changes, whichever process changed it (the
// process itself, another process through SetFrontProcess(), or the input focus moving). This turns
// each change into the Carbon events of the application class:
//   - kEventAppFrontSwitched, for every process, with kEventParamProcessID the new front process
//     (not sent when no process is known to be in front);
//   - kEventAppActivated for the process that becomes the front process and kEventAppDeactivated for
//     the one that stops being it, after the front switch.
// The events are sent to the application target on the thread that runs the main run loop, from the
// run loop, so that handlers run whether the program waits in RunApplicationEventLoop() or in
// another run loop (an AppKit application); when the change is made by this process itself the events
// are sent before the call that made it returns.

#include <HIToolbox/CarbonEventsCore.h>
#include <HIToolbox/CarbonEvents.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreGraphics/CoreGraphicsPrivate.h>
#include <dispatch/dispatch.h>
#include <unistd.h>
#include "HIToolboxInternal.h"

static void sendApplicationEvent(UInt32 kind, pid_t front)
{
	EventRef event;

	if (CreateEvent(NULL, kEventClassApplication, kind, 0, kEventAttributeNone, &event) != noErr)
		return;
	if (front > 0)
	{
		ProcessSerialNumber psn;

		if (GetProcessForPID(front, &psn) != noErr)
		{
			ReleaseEvent(event);
			return;
		}
		SetEventParameter(event, kEventParamProcessID, typeProcessSerialNumber, sizeof(psn), &psn);
	}
	SendEventToEventTarget(event, _HIApplicationEventTarget());
	ReleaseEvent(event);
}

static void frontProcessChanged(pid_t previous, pid_t current, void* info)
{
	pid_t self = getpid();

	if (current > 0)
		sendApplicationEvent(kEventAppFrontSwitched, current);
	if (current == self)
		sendApplicationEvent(kEventAppActivated, 0);
	else if (previous == self)
		sendApplicationEvent(kEventAppDeactivated, 0);
}

void _HIObserveFrontProcess(void)
{
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		_CGSAddFrontProcessObserver(frontProcessChanged, NULL);
	});
}
