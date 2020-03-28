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
#include "DefaultOutputAU.h"

#pragma GCC visibility push(default)
AUDIOCOMPONENT_ENTRY(AUOutputBaseFactory, DefaultOutputAU);
#pragma GCC visibility pop

enum {
	kOutputBus = 0,
	kInputBus
};

DefaultOutputAU::DefaultOutputAU(AudioComponentInstance inInstance)
: AUHAL(inInstance)
{
}

OSStatus DefaultOutputAU::GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, UInt32& outDataSize, Boolean& outWritable)
{
	OSStatus status = AUHAL::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);

	if (inID == kAudioOutputUnitProperty_CurrentDevice)
		outWritable = false;

	return status;
}

OSStatus DefaultOutputAU::SetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
	AudioUnitElement inElement, const void* inData, UInt32 inDataSize)
{
	switch (inID)
	{
		case kAudioOutputUnitProperty_CurrentDevice:
			return kAudioUnitErr_InvalidProperty;
	}
	return AUHAL::SetProperty(inID, inScope, inElement, inData, inDataSize);
}
