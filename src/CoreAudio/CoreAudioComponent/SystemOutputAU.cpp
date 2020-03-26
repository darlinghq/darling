/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

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
#include "SystemOutputAU.h"

#pragma GCC visibility push(default)
AUDIOCOMPONENT_ENTRY(AUOutputBaseFactory, SystemOutputAU);
#pragma GCC visibility pop

enum {
	kOutputBus = 0,
	kInputBus
};

SystemOutputAU::SystemOutputAU(AudioComponentInstance inInstance)
: AUHAL(inInstance)
{
	UInt32 propSize = sizeof(AudioDeviceID);
	AudioHardwareGetProperty(kAudioHardwarePropertyDefaultSystemOutputDevice, &propSize, &m_outputDevice);
}

OSStatus SystemOutputAU::SetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, const void* inData, UInt32 inDataSize)
{
	switch (inID)
	{
		case kAudioOutputUnitProperty_EnableIO:
		{
			if (inElement == kInputBus)
				return kAudioUnitErr_InvalidElement;
			break;
		}
		case kAudioOutputUnitProperty_CurrentDevice:
			return kAudioUnitErr_InvalidProperty;
	}
	return AUHAL::SetProperty(inID, inScope, inElement, inData, inDataSize);
}
