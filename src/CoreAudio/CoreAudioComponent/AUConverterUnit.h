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

#ifndef AUCONVERTERUNIT_H
#define AUCONVERTERUNIT_H
#include "AUBase.h"
#include <AudioToolbox/AudioConverter.h>

// 'aufc'/'conv'/'appl' -- generic format converter, wrapping the public
// AudioConverter API (AudioConverterNew/AudioConverterFillComplexBuffer)
// rather than reimplementing conversion.
class AUConverterUnit : public AUBase
{
public:
	AUConverterUnit(AudioComponentInstance inInstance);
	~AUConverterUnit();

	bool CanScheduleParameters() const override { return false; }
	bool StreamFormatWritable(AudioUnitScope scope, AudioUnitElement element) override { return !IsInitialized(); }

	OSStatus Initialize() override;
	void Cleanup() override;
	OSStatus Render(AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp, UInt32 inNumberFrames) override;

private:
	static OSStatus InputProc(AudioConverterRef inAudioConverter, UInt32* ioNumberDataPackets, AudioBufferList* ioData,
		AudioStreamPacketDescription** outDataPacketDescription, void* inUserData);

	AudioConverterRef m_converter = nullptr;
	AudioUnitRenderActionFlags* m_pullActionFlags = nullptr;
	AudioTimeStamp m_pullTimeStamp = {};
};

#endif /* AUCONVERTERUNIT_H */
