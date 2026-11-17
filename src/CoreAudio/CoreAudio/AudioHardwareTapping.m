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

// The process tap calls (macOS 14.2 and later). Darling's audio backend cannot capture the output of
// other processes, so no tap can be created: a creation request that is well formed is refused as
// an unsupported operation and the system's tap list stays empty.

#import <CoreAudio/CoreAudio.h>
#import <CoreAudio/AudioHardwareTapping.h>

OSStatus AudioHardwareCreateProcessTap(CATapDescription* inDescription, AudioObjectID* outTapID)
{
	// Without a description there is nothing to build a tap from. outTapID is left as it was.
	if (inDescription == nil)
		return kAudioHardwareIllegalOperationError;

	if (outTapID != NULL)
		*outTapID = kAudioObjectUnknown;
	return kAudioHardwareUnsupportedOperationError;
}

OSStatus AudioHardwareDestroyProcessTap(AudioObjectID inTapID)
{
	// QUIRK: reports success for any object ID, including ones that are not taps.
	// Evidence: black-box observation on macOS 26.7.3 (tier 2): kAudioObjectUnknown, an unused ID and the
	// system object all answer noErr. Affects every ID, no tap can exist here to be destroyed.
	// Test: coreaudio_process_tap (destroy of an unknown ID).
	return kAudioHardwareNoError;
}
