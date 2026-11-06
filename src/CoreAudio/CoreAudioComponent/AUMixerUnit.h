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

#ifndef AUMIXERUNIT_H
#define AUMIXERUNIT_H
#include "AUBase.h"
#include "MixerSum.h"
#include <vector>

// 'aumx'/'mcmx'/'appl' -- N-input summing mixer, per-input gain, one output.
class AUMixerUnit : public AUBase
{
public:
	AUMixerUnit(AudioComponentInstance inInstance, UInt32 numInputs = 8);

	bool CanScheduleParameters() const override { return false; }
	bool StreamFormatWritable(AudioUnitScope scope, AudioUnitElement element) override { return !IsInitialized(); }
	bool BusCountWritable(AudioUnitScope scope) override { return scope == kAudioUnitScope_Input; }
	OSStatus SetBusCount(AudioUnitScope scope, UInt32 count) override;

	OSStatus Render(AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp, UInt32 inNumberFrames) override;
	OSStatus SetParameter(AudioUnitParameterID inID, AudioUnitScope inScope, AudioUnitElement inElement,
		AudioUnitParameterValue inValue, UInt32 inBufferOffsetInFrames) override;
	OSStatus GetParameter(AudioUnitParameterID inID, AudioUnitScope inScope, AudioUnitElement inElement,
		AudioUnitParameterValue& outValue) override;

private:
	// Fixed size, as the render thread reads these while parameters change.
	enum { kMaxInputs = 64 };
	float m_gains[kMaxInputs];   // kMultiChannelMixerParam_Volume on input elements
	bool m_enabled[kMaxInputs];  // kMultiChannelMixerParam_Enable on input elements
	float m_pans[kMaxInputs];    // kMultiChannelMixerParam_Pan on input elements
	float m_outputGain = 1.0f;   // kMultiChannelMixerParam_Volume on the output element
};

#endif /* AUMIXERUNIT_H */
