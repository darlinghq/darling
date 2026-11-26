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

#include "GenericOutputAU.h"
#include <AudioToolbox/AudioOutputUnit.h>
#include <string.h>

#pragma GCC visibility push(default)
AUDIOCOMPONENT_ENTRY(AUOutputBaseFactory, GenericOutputAU);
#pragma GCC visibility pop

GenericOutputAU::GenericOutputAU(AudioComponentInstance inInstance)
: AUBase(inInstance, 1, 1)
{
}

GenericOutputAU::~GenericOutputAU()
{
	Cleanup();
}

void GenericOutputAU::Cleanup()
{
	if (m_converter)
	{
		AudioConverterDispose(m_converter);
		m_converter = nullptr;
	}
	m_running = false;
	AUBase::Cleanup();
}

static bool sameFormat(const AudioStreamBasicDescription& a, const AudioStreamBasicDescription& b)
{
	return a.mSampleRate == b.mSampleRate && a.mFormatID == b.mFormatID && a.mFormatFlags == b.mFormatFlags
		&& a.mBytesPerPacket == b.mBytesPerPacket && a.mFramesPerPacket == b.mFramesPerPacket
		&& a.mBytesPerFrame == b.mBytesPerFrame && a.mChannelsPerFrame == b.mChannelsPerFrame
		&& a.mBitsPerChannel == b.mBitsPerChannel;
}

OSStatus GenericOutputAU::Initialize()
{
	OSStatus s = AUBase::Initialize();
	if (s != noErr)
		return s;

	const CAStreamBasicDescription& in = GetStreamFormat(kAudioUnitScope_Input, 0);
	const CAStreamBasicDescription& out = GetStreamFormat(kAudioUnitScope_Output, 0);
	if (sameFormat(in, out))
		return noErr;
	return AudioConverterNew(&in, &out, &m_converter);
}

OSStatus GenericOutputAU::Start()
{
	// There is no device to start: the unit is "running" so a client that checks gets
	// the state it set, while rendering remains driven by AudioUnitRender().
	m_running = true;
	return noErr;
}

OSStatus GenericOutputAU::Stop()
{
	m_running = false;
	return noErr;
}

OSStatus GenericOutputAU::GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement,
	UInt32& outDataSize, Boolean& outWritable)
{
	if (inID == kAudioOutputUnitProperty_IsRunning)
	{
		if (inScope != kAudioUnitScope_Global)
			return kAudioUnitErr_InvalidScope;
		outDataSize = sizeof(UInt32);
		outWritable = false;
		return noErr;
	}
	return AUBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
}

OSStatus GenericOutputAU::GetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData)
{
	if (inID == kAudioOutputUnitProperty_IsRunning)
	{
		if (inScope != kAudioUnitScope_Global)
			return kAudioUnitErr_InvalidScope;
		*static_cast<UInt32*>(outData) = m_running ? 1 : 0;
		return noErr;
	}
	return AUBase::GetProperty(inID, inScope, inElement, outData);
}

OSStatus GenericOutputAU::InputProc(AudioConverterRef inAudioConverter, UInt32* ioNumberDataPackets, AudioBufferList* ioData,
	AudioStreamPacketDescription** outDataPacketDescription, void* inUserData)
{
	GenericOutputAU* self = static_cast<GenericOutputAU*>(inUserData);
	if (outDataPacketDescription)
		*outDataPacketDescription = nullptr;
	OSStatus s = self->GetInput(0)->PullInputWithBufferList(*self->m_pullActionFlags, self->m_pullTimeStamp, 0, *ioNumberDataPackets, ioData);
	if (s != noErr)
	{
		*ioNumberDataPackets = 0;
		return s;
	}
	const CAStreamBasicDescription& in = self->GetStreamFormat(kAudioUnitScope_Input, 0);
	if (ioData->mNumberBuffers > 0 && in.mBytesPerFrame > 0)
		*ioNumberDataPackets = ioData->mBuffers[0].mDataByteSize / in.mBytesPerFrame;
	return noErr;
}

OSStatus GenericOutputAU::Render(AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp, UInt32 inNumberFrames)
{
	if (!HasInput(0))
		return kAudioUnitErr_NoConnection;
	AudioBufferList& out = GetOutput(0)->GetBufferList();
	if (!m_converter)
	{
		// Same format on both sides: what the input supplies is the output.
		OSStatus s = GetInput(0)->PullInput(ioActionFlags, inTimeStamp, 0, inNumberFrames);
		if (s != noErr)
			return s;
		const AudioBufferList& in = GetInput(0)->GetBufferList();
		for (UInt32 b = 0; b < out.mNumberBuffers && b < in.mNumberBuffers; b++)
		{
			UInt32 n = in.mBuffers[b].mDataByteSize < out.mBuffers[b].mDataByteSize ? in.mBuffers[b].mDataByteSize : out.mBuffers[b].mDataByteSize;
			if (out.mBuffers[b].mData != in.mBuffers[b].mData)
				memcpy(out.mBuffers[b].mData, in.mBuffers[b].mData, n);
			out.mBuffers[b].mDataByteSize = n;
		}
		return noErr;
	}

	m_pullActionFlags = &ioActionFlags;
	m_pullTimeStamp = inTimeStamp;
	UInt32 packets = inNumberFrames;
	return AudioConverterFillComplexBuffer(m_converter, InputProc, this, &packets, &out, nullptr);
}
