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
#include <AudioToolbox/AudioUnitParameters.h>
#include <algorithm>
#include <cstring>
#include <vector>

#pragma GCC visibility push(default)
AUDIOCOMPONENT_ENTRY(AUBaseFactory, AUMixerUnit);
#pragma GCC visibility pop

AUMixerUnit::AUMixerUnit(AudioComponentInstance inInstance, UInt32 numInputs)
: AUBase(inInstance, std::min<UInt32>(numInputs, kMaxInputs), 1)
{
	std::fill(std::begin(m_gains), std::end(m_gains), 1.0f);
	std::fill(std::begin(m_enabled), std::end(m_enabled), true);
	std::fill(std::begin(m_pans), std::end(m_pans), 0.0f);
}

OSStatus AUMixerUnit::SetBusCount(AudioUnitScope scope, UInt32 count)
{
	if (count > kMaxInputs)
		return kAudioUnitErr_InvalidPropertyValue;
	return AUBase::SetBusCount(scope, count);
}

OSStatus AUMixerUnit::Render(AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp, UInt32 inNumberFrames)
{
	AudioBufferList& out = GetOutput(0)->GetBufferList();
	std::vector<const AudioBufferList*> inputs;
	std::vector<UInt32> inputIds;

	UInt32 numInputs = std::min<UInt32>(Inputs().GetNumberOfElements(), kMaxInputs);
	for (UInt32 i = 0; i < numInputs; i++)
	{
		if (!HasInput(i) || !m_enabled[i])
			continue;

		AudioUnitRenderActionFlags flags = 0;
		if (GetInput(i)->PullInput(flags, inTimeStamp, i, inNumberFrames) != noErr)
			continue;

		inputs.push_back(&GetInput(i)->GetBufferList());
		inputIds.push_back(i);
	}

	if (inputs.empty())
	{
		for (UInt32 c = 0; c < out.mNumberBuffers; c++)
			std::memset(out.mBuffers[c].mData, 0, out.mBuffers[c].mDataByteSize);
		ioActionFlags |= kAudioUnitRenderAction_OutputIsSilence;
		return noErr;
	}

	// Canonical audio has one buffer per channel. A mono input feeds every output channel.
	std::vector<const float*> channelInputs(inputs.size());
	std::vector<float> channelGains(inputs.size());

	for (UInt32 c = 0; c < out.mNumberBuffers; c++)
	{
		for (size_t k = 0; k < inputs.size(); k++)
		{
			const UInt32 buffer = std::min(c, inputs[k]->mNumberBuffers - 1);
			const float pan = m_pans[inputIds[k]];

			// Only stereo output has a left and a right to pan between.
			float panGain = 1.0f;
			if (out.mNumberBuffers == 2)
				panGain = c == 0 ? std::min(1.0f, 1.0f - pan) : std::min(1.0f, 1.0f + pan);

			channelInputs[k] = static_cast<const float*>(inputs[k]->mBuffers[buffer].mData);
			channelGains[k] = m_gains[inputIds[k]] * panGain * m_outputGain;
		}

		mixerSum(channelInputs.data(), channelGains.data(), inputs.size(), static_cast<float*>(out.mBuffers[c].mData), inNumberFrames);
	}

	ioActionFlags &= ~kAudioUnitRenderAction_OutputIsSilence;
	return noErr;
}

OSStatus AUMixerUnit::SetParameter(AudioUnitParameterID inID, AudioUnitScope inScope, AudioUnitElement inElement,
	AudioUnitParameterValue inValue, UInt32 inBufferOffsetInFrames)
{
	if (inScope == kAudioUnitScope_Output && inID == kMultiChannelMixerParam_Volume)
	{
		m_outputGain = inValue;
		return noErr;
	}
	if (inScope != kAudioUnitScope_Input)
		return kAudioUnitErr_InvalidScope;
	if (inElement >= kMaxInputs)
		return kAudioUnitErr_InvalidElement;

	switch (inID)
	{
		case kMultiChannelMixerParam_Volume:
			m_gains[inElement] = inValue;
			return noErr;
		case kMultiChannelMixerParam_Enable:
			m_enabled[inElement] = inValue != 0;
			return noErr;
		case kMultiChannelMixerParam_Pan:
			m_pans[inElement] = std::clamp(inValue, -1.0f, 1.0f);
			return noErr;
	}
	return kAudioUnitErr_InvalidParameter;
}

OSStatus AUMixerUnit::GetParameter(AudioUnitParameterID inID, AudioUnitScope inScope, AudioUnitElement inElement,
	AudioUnitParameterValue& outValue)
{
	if (inScope == kAudioUnitScope_Output && inID == kMultiChannelMixerParam_Volume)
	{
		outValue = m_outputGain;
		return noErr;
	}
	if (inScope != kAudioUnitScope_Input)
		return kAudioUnitErr_InvalidScope;
	if (inElement >= kMaxInputs)
		return kAudioUnitErr_InvalidElement;

	switch (inID)
	{
		case kMultiChannelMixerParam_Volume:
			outValue = m_gains[inElement];
			return noErr;
		case kMultiChannelMixerParam_Enable:
			outValue = m_enabled[inElement] ? 1.0f : 0.0f;
			return noErr;
		case kMultiChannelMixerParam_Pan:
			outValue = m_pans[inElement];
			return noErr;
	}
	return kAudioUnitErr_InvalidParameter;
}
