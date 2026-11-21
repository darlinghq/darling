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

#ifdef __OBJC__

#ifndef AudioHardwareTapping_h
#define AudioHardwareTapping_h

#include <Availability.h>
#include <CoreAudio/AudioHardware.h>

@class CATapDescription;

#if defined(__cplusplus)
extern "C"
{
#endif

// Makes a process tap from a description. On success *outTapID names the new tap object.
OSStatus AudioHardwareCreateProcessTap(CATapDescription* inDescription, AudioObjectID* outTapID);

// Destroys a process tap made by AudioHardwareCreateProcessTap.
OSStatus AudioHardwareDestroyProcessTap(AudioObjectID inTapID);

#if defined(__cplusplus)
}
#endif

#endif /* AudioHardwareTapping_h */

#endif /* __OBJC__ */
