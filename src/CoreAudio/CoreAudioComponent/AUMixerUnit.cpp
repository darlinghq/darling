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

#include "AUMixerUnit.h"
#include <algorithm>
#include <cstring>

#pragma GCC visibility push(default)
AUDIOCOMPONENT_ENTRY(AUBaseFactory, AUMixerUnit);
#pragma GCC visibility pop

AUMixerUnit::AUMixerUnit(AudioComponentInstance inInstance, UInt32 numInputs)
: AUBase(inInstance, numInputs, 1)
, m_gains(numInputs, 1.0f)
{
}

OSStatus AUMixerUnit::Render(AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp, UInt32 inNumberFrames)
{
	AudioBufferList& out = GetOutput(0)->GetBufferList();
	std::vector<const float*> inputPtrs;
	std::vector<float> gains;

	UInt32 numInputs = Inputs().GetNumberOfElements();
	for (UInt32 i = 0; i < numInputs; i++)
	{
		if (!HasInput(i))
			continue;
		OSStatus s = GetInput(i)->PullInput(ioActionFlags, inTimeStamp, i, inNumberFrames);
		if (s != noErr)
			continue;
		inputPtrs.push_back(static_cast<const float*>(GetInput(i)->GetBufferList().mBuffers[0].mData));
		gains.push_back(i < m_gains.size() ? m_gains[i] : 1.0f);
	}

	if (inputPtrs.empty())
	{
		std::memset(out.mBuffers[0].mData, 0, out.mBuffers[0].mDataByteSize);
		return noErr;
	}

	mixerSum(inputPtrs.data(), gains.data(), inputPtrs.size(), static_cast<float*>(out.mBuffers[0].mData), inNumberFrames);
	return noErr;
}

OSStatus AUMixerUnit::SetParameter(AudioUnitParameterID inID, AudioUnitScope inScope, AudioUnitElement inElement,
	AudioUnitParameterValue inValue, UInt32 inBufferOffsetInFrames)
{
	if (inScope == kAudioUnitScope_Input && inElement < m_gains.size())
	{
		m_gains[inElement] = inValue;
		return noErr;
	}
	return AUBase::SetParameter(inID, inScope, inElement, inValue, inBufferOffsetInFrames);
}

OSStatus AUMixerUnit::GetParameter(AudioUnitParameterID inID, AudioUnitScope inScope, AudioUnitElement inElement,
	AudioUnitParameterValue& outValue)
{
	if (inScope == kAudioUnitScope_Input && inElement < m_gains.size())
	{
		outValue = m_gains[inElement];
		return noErr;
	}
	return AUBase::GetParameter(inID, inScope, inElement, outValue);
}
