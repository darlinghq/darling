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

#include "AUConverterUnit.h"

#pragma GCC visibility push(default)
AUDIOCOMPONENT_ENTRY(AUBaseFactory, AUConverterUnit);
#pragma GCC visibility pop

AUConverterUnit::AUConverterUnit(AudioComponentInstance inInstance)
: AUBase(inInstance, 1, 1)
{
}

AUConverterUnit::~AUConverterUnit()
{
	Cleanup();
}

void AUConverterUnit::Cleanup()
{
	if (m_converter)
	{
		AudioConverterDispose(m_converter);
		m_converter = nullptr;
	}
	AUBase::Cleanup();
}

OSStatus AUConverterUnit::Initialize()
{
	OSStatus s = AUBase::Initialize();
	if (s != noErr)
		return s;

	const CAStreamBasicDescription& in = GetStreamFormat(kAudioUnitScope_Input, 0);
	const CAStreamBasicDescription& out = GetStreamFormat(kAudioUnitScope_Output, 0);

	return AudioConverterNew(&in, &out, &m_converter);
}

OSStatus AUConverterUnit::InputProc(AudioConverterRef inAudioConverter, UInt32* ioNumberDataPackets, AudioBufferList* ioData,
	AudioStreamPacketDescription** outDataPacketDescription, void* inUserData)
{
	AUConverterUnit* self = static_cast<AUConverterUnit*>(inUserData);

	if (outDataPacketDescription)
		*outDataPacketDescription = nullptr;

	OSStatus s = self->GetInput(0)->PullInputWithBufferList(*self->m_pullActionFlags, self->m_pullTimeStamp, 0, *ioNumberDataPackets, ioData);
	if (s != noErr)
	{
		*ioNumberDataPackets = 0;
		return s;
	}

	// PullInputWithBufferList fills in actual byte counts, which may be less
	// than requested (e.g. at the end of a finite source); report that back
	// as the actual packet (frame) count, not what was asked for.
	const CAStreamBasicDescription& in = self->GetStreamFormat(kAudioUnitScope_Input, 0);
	if (ioData->mNumberBuffers > 0 && in.mBytesPerFrame > 0)
		*ioNumberDataPackets = ioData->mBuffers[0].mDataByteSize / in.mBytesPerFrame;

	return noErr;
}

OSStatus AUConverterUnit::Render(AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp, UInt32 inNumberFrames)
{
	if (!m_converter)
		return kAudioUnitErr_Uninitialized;

	AudioBufferList& outBuffer = GetOutput(0)->GetBufferList();

	// Stashed for InputProc(), which AudioConverterFillComplexBuffer calls
	// (possibly more than once) to pull as much upstream data as it needs.
	m_pullActionFlags = &ioActionFlags;
	m_pullTimeStamp = inTimeStamp;

	UInt32 packetSize = inNumberFrames;
	return AudioConverterFillComplexBuffer(m_converter, InputProc, this, &packetSize, &outBuffer, nullptr);
}
