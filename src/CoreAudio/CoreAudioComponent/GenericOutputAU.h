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

#ifndef _CA_GENERIC_OUTPUT_AU_H
#define _CA_GENERIC_OUTPUT_AU_H
#include "AUBase.h"
#include <AudioToolbox/AudioConverter.h>

// 'auou'/'genr'/'appl' -- an output unit without a device. It does not run on a clock of
// its own: whoever calls AudioUnitRender() on its output bus drives the whole chain that
// feeds it, which is how audio is rendered offline. Audio from the input bus is passed on
// unchanged, or converted when the input and output formats differ.
class GenericOutputAU : public AUBase
{
public:
	GenericOutputAU(AudioComponentInstance inInstance);
	~GenericOutputAU();
	bool CanScheduleParameters() const override { return false; }
	bool StreamFormatWritable(AudioUnitScope scope, AudioUnitElement element) override { return !IsInitialized(); }
	OSStatus Initialize() override;
	void Cleanup() override;
	OSStatus Start() override;
	OSStatus Stop() override;
	OSStatus GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement,
		UInt32& outDataSize, Boolean& outWritable) override;
	OSStatus GetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData) override;
	OSStatus Render(AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp, UInt32 inNumberFrames) override;
private:
	static OSStatus InputProc(AudioConverterRef inAudioConverter, UInt32* ioNumberDataPackets, AudioBufferList* ioData,
		AudioStreamPacketDescription** outDataPacketDescription, void* inUserData);

	bool m_running = false;
	AudioConverterRef m_converter = nullptr;
	AudioUnitRenderActionFlags* m_pullActionFlags = nullptr;
	AudioTimeStamp m_pullTimeStamp = {};
};

#endif
