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

#ifndef HITOOLBOX_INTERNAL_H
#define HITOOLBOX_INTERNAL_H

// Not part of the public headers.
//
// The Carbon events made from CoreGraphics events (CreateEventWithCGEvent) have this class, and the
// CGEventType as their kind; kind 14 is a system-defined event. Observed on macOS 26.7.3 (tier 2,
// mac-probes/v443-systemdefined-carbon-event.m, MC-V443-APPKIT-IMPORTS in MAC-EVIDENCE.md).
#define kHIEventClassRaw 'cgs '
#define kHIEventRawSystemDefined 14

#endif
