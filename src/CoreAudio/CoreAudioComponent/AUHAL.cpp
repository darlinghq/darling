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

#include "AUHAL.h"
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <mach/mach_time.h>
#include <vector>

#pragma GCC visibility push(default)
AUDIOCOMPONENT_ENTRY(AUOutputBaseFactory, AUHAL);
#pragma GCC visibility pop

enum {
	kOutputBus = 0,
	kInputBus
};

// Element 0 is the output (app -> device) side and element 1 the input (device -> app)
// side, on both scopes. The app's recording format and AudioUnitRender(bus 1) use output element 1.
AUHAL::AUHAL(AudioComponentInstance inInstance, bool supportRecording)
: AUBase(inInstance, supportRecording ? 2 : 1, supportRecording ? 2 : 1)
{
	UInt32 propSize = sizeof(AudioDeviceID);
	AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &propSize, &m_outputDevice);
	AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &propSize, &m_inputDevice);
}

AUHAL::~AUHAL()
{
	Stop();
}

UInt32 AUHAL::GetChannelLayoutTags(AudioUnitScope scope, AudioUnitElement element, AudioChannelLayoutTag* tags)
{
	if (element != kOutputBus || (scope != kAudioUnitScope_Input && scope != kAudioUnitScope_Output))
		return 0;

	const UInt32 channels = GetStreamFormat(scope, element).mChannelsPerFrame;
	if (tags)
		*tags = channels == 1 ? kAudioChannelLayoutTag_Mono : channels == 2 ? kAudioChannelLayoutTag_Stereo : (kAudioChannelLayoutTag_DiscreteInOrder | channels);
	return 1;
}

UInt32 AUHAL::GetAudioChannelLayout(AudioUnitScope scope, AudioUnitElement element, AudioChannelLayout* layout, Boolean& writable)
{
	AudioChannelLayoutTag tag;
	if (!GetChannelLayoutTags(scope, element, &tag))
		return 0;

	writable = !m_running;
	if (layout)
	{
		layout->mChannelLayoutTag = tag;
		layout->mChannelBitmap = 0;
		layout->mNumberChannelDescriptions = 0;
	}
	return offsetof(AudioChannelLayout, mChannelDescriptions);
}

OSStatus AUHAL::SetAudioChannelLayout(AudioUnitScope scope, AudioUnitElement element, const AudioChannelLayout* layout)
{
	AudioChannelLayoutTag tag;
	if (!layout || !GetChannelLayoutTags(scope, element, &tag))
		return kAudioUnitErr_InvalidElement;
	if (layout->mChannelLayoutTag == tag)
		return noErr;

	// A layout is acceptable when it describes the stream's number of channels, whichever
	// way it is written: channel descriptions, a channel bitmap or another tag with that count.
	UInt32 channels;
	if (layout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelDescriptions)
		channels = layout->mNumberChannelDescriptions;
	else if (layout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelBitmap)
		channels = __builtin_popcount(layout->mChannelBitmap);
	else
		channels = layout->mChannelLayoutTag & 0xffff;

	return channels == GetStreamFormat(scope, element).mChannelsPerFrame ? noErr : kAudioUnitErr_InvalidPropertyValue;
}

bool AUHAL::CanScheduleParameters() const
{
	return true;
}

bool AUHAL::StreamFormatWritable(AudioUnitScope scope, AudioUnitElement element)
{
	return !m_running;
}

OSStatus AUHAL::Version()
{
	return 1;
}

// Provide data from the microphone: AudioUnitRender(bus 1), normally called from the input callback.
// Never blocks; frames not recorded yet are silence. Bypasses NeedsToRender(), as the client
// decides which timestamp to render.
OSStatus AUHAL::RenderBus(AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames)
{
	if (inBusNumber != kInputBus || !m_enableInput)
		return kAudioUnitErr_NoConnection;

	const CAStreamBasicDescription& desc = GetStreamFormat(kAudioUnitScope_Output, kInputBus);
	const UInt32 sampleBytes = desc.mBitsPerChannel / 8;
	const UInt32 frameBytes = desc.mChannelsPerFrame * sampleBytes;
	if (!frameBytes)
		return kAudioUnitErr_FormatNotSupported;

	AudioBufferList& abl = GetOutput(kInputBus)->GetBufferList();

	if (!(desc.mFormatFlags & kAudioFormatFlagIsNonInterleaved))
	{
		uint8_t* destination = static_cast<uint8_t*>(abl.mBuffers[0].mData);
		size_t frames = m_recordRingBuffer ? m_recordRingBuffer->read(destination, inNumberFrames) : 0;

		memset(destination + frames * frameBytes, 0, (inNumberFrames - frames) * frameBytes);
		abl.mBuffers[0].mDataByteSize = inNumberFrames * frameBytes;
		return noErr;
	}

	if (m_recordScratch.size() < size_t(inNumberFrames) * frameBytes)
		m_recordScratch.resize(size_t(inNumberFrames) * frameBytes);

	const size_t frames = m_recordRingBuffer ? m_recordRingBuffer->read(m_recordScratch.data(), inNumberFrames) : 0;

	for (UInt32 c = 0; c < abl.mNumberBuffers && c < desc.mChannelsPerFrame; c++)
	{
		uint8_t* destination = static_cast<uint8_t*>(abl.mBuffers[c].mData);
		for (size_t f = 0; f < frames; f++)
			memcpy(destination + f * sampleBytes, m_recordScratch.data() + f * frameBytes + c * sampleBytes, sampleBytes);

		memset(destination + frames * sampleBytes, 0, (inNumberFrames - frames) * sampleBytes);
		abl.mBuffers[c].mDataByteSize = inNumberFrames * sampleBytes;
	}
	return noErr;
}

OSStatus AUHAL::ChangeStreamFormat(AudioUnitScope inScope, AudioUnitElement inElement,
	const CAStreamBasicDescription& inPrevFormat, const CAStreamBasicDescription& inNewFormat)
{
	OSStatus result = AUBase::ChangeStreamFormat(inScope, inElement, inPrevFormat, inNewFormat);

	if (result == noErr && inScope == kAudioUnitScope_Output && inElement == kInputBus)
	{
		// The old ring buffer was sized/channeled for inPrevFormat; it must
		// not be reused under a different channel count. Only reachable
		// while stopped (StreamFormatWritable() requires !m_running), so
		// there's no concurrent Render()/doRecord() to race against here.
		m_recordRingBuffer.reset();
	}

	return result;
}

bool AUHAL::ValidFormat(AudioUnitScope inScope, AudioUnitElement inElement, const CAStreamBasicDescription& inNewFormat)
{
	const bool clientSide = (inScope == kAudioUnitScope_Output && inElement == kInputBus)
		|| (inScope == kAudioUnitScope_Input && inElement == kOutputBus);
	if (!clientSide)
		return AUBase::ValidFormat(inScope, inElement, inNewFormat);

	// Native-endian float32 or signed 16/32-bit integers at any rate, which PulseAudio converts.
	// Missing frames are padded with zeros, which is only silence for float and signed samples.
	// 24-bit is left out, as PulseAudio may use 4-byte samples where our copies use 3.
	const UInt32 flags = inNewFormat.mFormatFlags;
	const UInt32 bits = inNewFormat.mBitsPerChannel;
	if (inNewFormat.mFormatID != kAudioFormatLinearPCM || inNewFormat.mFramesPerPacket != 1
		|| inNewFormat.mSampleRate <= 0 || inNewFormat.mChannelsPerFrame < 1 || inNewFormat.mChannelsPerFrame > 32
		|| (flags & kAudioFormatFlagIsBigEndian) != (kAudioFormatFlagsNativeEndian & kAudioFormatFlagIsBigEndian))
		return false;

	if ((flags & kAudioFormatFlagIsFloat) ? bits != 32 : (!(flags & kAudioFormatFlagIsSignedInteger) || (bits != 16 && bits != 32)))
		return false;

	const UInt32 frameBytes = (flags & kAudioFormatFlagIsNonInterleaved) ? bits / 8 : inNewFormat.mChannelsPerFrame * bits / 8;
	return inNewFormat.mBytesPerFrame == frameBytes && inNewFormat.mBytesPerPacket == frameBytes;
}

// The device handles interleaved data only and PulseAudio converts rate, channels and sample
// type, so it runs directly in the client's format. Keeps the device's IO cycle length,
// as the device stores its buffer size in bytes.
static OSStatus setDeviceFormat(AudioDeviceID device, bool isInput, const CAStreamBasicDescription& desc)
{
	AudioStreamBasicDescription hardware = desc;
	hardware.mFormatFlags &= ~kAudioFormatFlagIsNonInterleaved;
	hardware.mBytesPerFrame = hardware.mChannelsPerFrame * hardware.mBitsPerChannel / 8;
	hardware.mBytesPerPacket = hardware.mBytesPerFrame;

	AudioObjectPropertyAddress address = { kAudioDevicePropertyBufferFrameSize,
		isInput ? kAudioObjectPropertyScopeInput : kAudioObjectPropertyScopeOutput, kAudioObjectPropertyElementMaster };
	UInt32 frames = 0, size = sizeof(frames);
	const bool haveFrames = AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &frames) == noErr;

	OSStatus status = AudioDeviceSetProperty(device, nullptr, 0, isInput, kAudioDevicePropertyStreamFormat, sizeof(hardware), &hardware);
	if (status == noErr && haveFrames)
		AudioObjectSetPropertyData(device, &address, 0, nullptr, sizeof(frames), &frames);
	return status;
}

OSStatus AUHAL::Start()
{
	if (m_running)
		return noErr;

	if (m_enableOutput)
	{
		// std::cout << "Output is enabled, starting playback\n";
		OSStatus status = AudioDeviceCreateIOProcID(m_outputDevice, playbackCallback, this, &m_outputProcID);
		if (status != noErr)
			return status;

		// m_auhalData.open("/tmp/auhal.raw", std::ios_base::binary | std::ios_base::out);

		status = setDeviceFormat(m_outputDevice, false, GetStreamFormat(kAudioUnitScope_Input, kOutputBus));
		if (status == noErr)
			status = AudioDeviceStart(m_outputDevice, m_outputProcID);
		if (status != noErr)
		{
			AudioDeviceDestroyIOProcID(m_outputDevice, m_outputProcID);
			m_outputProcID = 0;
			return status;
		}
	}
	if (m_enableInput)
	{
		const CAStreamBasicDescription& desc = GetStreamFormat(kAudioUnitScope_Output, kInputBus);

		// Constructed here (only reachable while !m_running, i.e. before
		// doRecord()/RenderBus() can run concurrently on separate threads) so
		// RenderBus() never observes a partially-published pointer -- doRecord()
		// only ever writes into an already-constructed buffer.
		const size_t capacityFrames = std::max<size_t>(desc.mSampleRate / 10, size_t(GetMaxFramesPerSlice()) * 2); // at least 100ms
		m_recordRingBuffer = std::make_unique<AURingBuffer>(capacityFrames, desc.mChannelsPerFrame, desc.mBitsPerChannel / 8);

		OSStatus status = AudioDeviceCreateIOProcID(m_inputDevice, recordCallback, this, &m_inputProcID);
		if (status == noErr)
		{
			status = setDeviceFormat(m_inputDevice, true, desc);
			if (status == noErr)
				status = AudioDeviceStart(m_inputDevice, m_inputProcID);
			if (status != noErr)
			{
				AudioDeviceDestroyIOProcID(m_inputDevice, m_inputProcID);
				m_inputProcID = 0;
			}
		}
		if (status != noErr)
		{
			if (m_outputProcID)
			{
				AudioDeviceStop(m_outputDevice, m_outputProcID);
				AudioDeviceDestroyIOProcID(m_outputDevice, m_outputProcID);
				m_outputProcID = 0;
			}
			return status;
		}
	}

	m_running = m_enableOutput || m_enableInput;
	return noErr;
}

OSStatus AUHAL::Stop()
{
	if (!m_running)
		return noErr;

	if (m_outputProcID)
	{
		AudioDeviceStop(m_outputDevice, m_outputProcID);
		AudioDeviceDestroyIOProcID(m_outputDevice, m_outputProcID);
		m_outputProcID = 0;
	}
	if (m_inputProcID)
	{
		AudioDeviceStop(m_inputDevice, m_inputProcID);
		AudioDeviceDestroyIOProcID(m_inputDevice, m_inputProcID);
		m_inputProcID = 0;
	}

	m_running = false;
	return noErr;
}

OSStatus AUHAL::SetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, const void* inData, UInt32 inDataSize)
{
	switch (inID)
	{
		case kAudioDevicePropertyBufferFrameSize:
		{
			// An input-only unit sizes its input device's IO cycle.
			AudioObjectPropertyAddress address = { inID, kAudioObjectPropertyScopeOutput, kAudioObjectPropertyElementMaster };
			OSStatus status = noErr;

			if (m_enableOutput || !m_enableInput)
				status = AudioObjectSetPropertyData(m_outputDevice, &address, 0, nullptr, inDataSize, inData);
			if (status == noErr && m_enableInput)
			{
				address.mScope = kAudioObjectPropertyScopeInput;
				status = AudioObjectSetPropertyData(m_inputDevice, &address, 0, nullptr, inDataSize, inData);
			}
			return status;
		}
		case kAudioOutputUnitProperty_SetInputCallback:
		{
			ca_require(inDataSize == sizeof(AURenderCallbackStruct), InvalidPropertyValue);
			const AURenderCallbackStruct* cb = static_cast<const AURenderCallbackStruct*>(inData);
			
			m_outputAvailableCb = *cb;

			PropertyChanged(inID, inScope, inElement);
			return noErr;
		}
		case kAudioOutputUnitProperty_EnableIO:
		{
			ca_require(inDataSize == sizeof(UInt32), InvalidPropertyValue);

			const bool enable = *((const UInt32*) inData);

			if (inElement == kOutputBus)
			{
				m_enableOutput = enable;
				PropertyChanged(inID, inScope, inElement);
			}
			else if (inElement == kInputBus)
			{
				m_enableInput = enable;
				PropertyChanged(inID, inScope, inElement);
			}
			else
				return kAudioUnitErr_InvalidElement;

			return noErr;
		}
		case kAudioOutputUnitProperty_CurrentDevice:
		{
			ca_require(inDataSize == sizeof(AudioDeviceID), InvalidPropertyValue);
			const AudioDeviceID* dev = static_cast<const AudioDeviceID*>(inData);

			// A unit has one current device, normally set on element 0.
			// For an input-only unit that is the capture device.
			if (inElement == kOutputBus && m_enableInput && !m_enableOutput)
			{
				m_inputDevice = *dev;
				PropertyChanged(inID, inScope, inElement);
			}
			else if (inElement == kOutputBus)
			{
				m_outputDevice = *dev;
				PropertyChanged(inID, inScope, inElement);
			}
			else if (inElement == kInputBus)
			{
				m_inputDevice = *dev;
				PropertyChanged(inID, inScope, inElement);
			}
			else
				return kAudioUnitErr_InvalidElement;
			return noErr;
		}
	}
	return AUBase::SetProperty(inID, inScope, inElement, inData, inDataSize);
InvalidPropertyValue:
	return kAudioUnitErr_InvalidPropertyValue;
}

OSStatus AUHAL::GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, UInt32& outDataSize, Boolean& outWritable)
{
	switch (inID)
	{
		case kAudioDevicePropertyBufferFrameSize:
		{
			outDataSize = sizeof(UInt32);
			outWritable = true;
			return noErr;
		}
		case kAudioOutputUnitProperty_SetInputCallback:
		{
			outDataSize = sizeof(AURenderCallbackStruct);
			outWritable = true;
			return noErr;
		}
		case kAudioOutputUnitProperty_EnableIO:
		{
			outDataSize = sizeof(UInt32);
			outWritable = true;
			return noErr;
		}
		case kAudioOutputUnitProperty_HasIO:
		{
			outDataSize = sizeof(UInt32);
			outWritable = false;
			return noErr;
		}
		case kAudioOutputUnitProperty_CurrentDevice:
		{
			outDataSize = sizeof(AudioDeviceID);
			outWritable = true;
			return noErr;
		}
	}
	return AUBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
}

OSStatus AUHAL::GetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData)
{
	switch (inID)
	{
		case kAudioDevicePropertyBufferFrameSize:
		{
			const bool input = m_enableInput && !m_enableOutput;
			AudioObjectPropertyAddress address = { inID, input ? kAudioObjectPropertyScopeInput : kAudioObjectPropertyScopeOutput, kAudioObjectPropertyElementMaster };
			UInt32 size = sizeof(UInt32);
			return AudioObjectGetPropertyData(input ? m_inputDevice : m_outputDevice, &address, 0, nullptr, &size, outData);
		}
		case kAudioOutputUnitProperty_SetInputCallback:
		{
			memcpy(outData, &m_outputAvailableCb, sizeof(m_outputAvailableCb));
			return noErr;
		}
		case kAudioOutputUnitProperty_EnableIO:
		case kAudioOutputUnitProperty_HasIO:
		{
			if (inElement == kOutputBus)
			{
				UInt32 value = m_enableOutput;
				memcpy(outData, &value, sizeof(value));
			}
			else if (inElement == kInputBus)
			{
				UInt32 value = m_enableInput;
				memcpy(outData, &value, sizeof(value));
			}
			else
				return kAudioUnitErr_InvalidElement;
			return noErr;
		}
		case kAudioOutputUnitProperty_CurrentDevice:
		{
			if (inElement == kOutputBus && m_enableInput && !m_enableOutput)
			{
				memcpy(outData, &m_inputDevice, sizeof(m_inputDevice));
			}
			else if (inElement == kOutputBus)
			{
				memcpy(outData, &m_outputDevice, sizeof(m_outputDevice));
			}
			else if (inElement == kInputBus)
			{
				memcpy(outData, &m_inputDevice, sizeof(m_inputDevice));
			}
			else
				return kAudioUnitErr_InvalidElement;
			return noErr;
		}
	}
	return AUBase::GetProperty(inID, inScope, inElement, outData);
}

OSStatus AUHAL::playbackCallback(AudioObjectID inObjectID,
	const AudioTimeStamp* inNow, const AudioBufferList* inInputData,
	const AudioTimeStamp* inInputTime,
	AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime,
	void* inClientData)
{
	AUHAL* This = static_cast<AUHAL*>(inClientData);
	return This->doPlayback(inNow, outOutputData, inOutputTime);
}

OSStatus AUHAL::recordCallback(AudioObjectID inObjectID,
	const AudioTimeStamp* inNow, const AudioBufferList* inInputData,
	const AudioTimeStamp* inInputTime,
	AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime,
	void* inClientData)
{
	AUHAL* This = static_cast<AUHAL*>(inClientData);
	return This->doRecord(inNow, inInputData, inInputTime);
}

OSStatus AUHAL::doPlayback(const AudioTimeStamp* inNow, AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime)
{
	// std::cout << "AUHAL::DoPlayback()\n";
	if (!HasInput(0))
	{
		// std::cerr << "No connection\n";
		return kAudioUnitErr_NoConnection;
	}

	OSStatus result = noErr;
	AudioUnitRenderActionFlags flags = kAudioUnitRenderAction_PreRender;
	const CAStreamBasicDescription& desc = GetStreamFormat(kAudioUnitScope_Input, kOutputBus);

	const UInt32 sampleBytes = desc.mBitsPerChannel / 8;
	const UInt32 frameBytes = desc.mChannelsPerFrame * sampleBytes;
	if (outOutputData->mNumberBuffers != 1 || !frameBytes || desc.mBitsPerChannel % 8)
		return kAudioUnitErr_FormatNotSupported;

	const UInt32 nFrames = outOutputData->mBuffers[0].mDataByteSize / frameBytes;

	if (desc.mFormatFlags & kAudioFormatFlagIsNonInterleaved)
	{
		// The device takes interleaved data. Zeroed so that samples a callback leaves unwritten are silence.
		m_playbackSamples.assign(size_t(nFrames) * frameBytes, 0);
		m_playbackBufferList.resize(offsetof(AudioBufferList, mBuffers) + desc.mChannelsPerFrame * sizeof(AudioBuffer));

		AudioBufferList* planar = reinterpret_cast<AudioBufferList*>(m_playbackBufferList.data());
		planar->mNumberBuffers = desc.mChannelsPerFrame;
		for (UInt32 c = 0; c < desc.mChannelsPerFrame; c++)
		{
			planar->mBuffers[c].mNumberChannels = 1;
			planar->mBuffers[c].mDataByteSize = nFrames * sampleBytes;
			planar->mBuffers[c].mData = m_playbackSamples.data() + size_t(c) * nFrames * sampleBytes;
		}

		result = GetInput(kOutputBus)->PullInputWithBufferList(flags, *inNow, kOutputBus, nFrames, planar);

		if (result == noErr)
		{
			uint8_t* destination = static_cast<uint8_t*>(outOutputData->mBuffers[0].mData);
			for (UInt32 f = 0; f < nFrames; f++)
			{
				for (UInt32 c = 0; c < desc.mChannelsPerFrame; c++)
				{
					memcpy(destination + size_t(f) * frameBytes + c * sampleBytes,
						static_cast<const uint8_t*>(planar->mBuffers[c].mData) + size_t(f) * sampleBytes, sampleBytes);
				}
			}
		}
	}
	else
		result = GetInput(kOutputBus)->PullInputWithBufferList(flags, *inNow, kOutputBus, nFrames, outOutputData);

	// std::cout << "Pull result: " << result << std::endl;
	// std::cout << "Bytes: " << outOutputData->mBuffers[0].mDataByteSize << std::endl;
	// m_auhalData.write((char*) outOutputData->mBuffers[0].mData, outOutputData->mBuffers[0].mDataByteSize);
	// m_auhalData.flush();

	return result;
}

OSStatus AUHAL::doRecord(const AudioTimeStamp* inNow, const AudioBufferList* inInputData, const AudioTimeStamp* inInputTime)
{
	// The device records interleaved data in the bus 1 output format (see Start()).
	const CAStreamBasicDescription& desc = GetStreamFormat(kAudioUnitScope_Output, kInputBus);
	const UInt32 frameBytes = desc.mChannelsPerFrame * (desc.mBitsPerChannel / 8);

	// Start() constructs m_recordRingBuffer before the IOProc that calls this
	// is ever installed; this should never be null here.
	if (!m_recordRingBuffer || !inInputData || !inInputData->mNumberBuffers || !frameBytes)
		return noErr;

	const uint8_t* source = static_cast<const uint8_t*>(inInputData->mBuffers[0].mData);
	const UInt32 totalFrames = inInputData->mBuffers[0].mDataByteSize / frameBytes;

	// AudioUnitRender refuses more than kAudioUnitProperty_MaximumFramesPerSlice frames.
	const UInt32 maxFrames = std::max<UInt32>(1, GetMaxFramesPerSlice());

	static mach_timebase_info_data_t timebase = {0, 0};
	if (!timebase.denom)
		mach_timebase_info(&timebase);

	for (UInt32 offset = 0; offset < totalFrames; )
	{
		const UInt32 frames = std::min(maxFrames, totalFrames - offset);

		// If RenderBus() isn't pulling fast enough, the frames that don't fit are dropped
		// rather than growing the buffer or blocking this (PulseAudio) thread.
		m_recordRingBuffer->write(source + size_t(offset) * frameBytes, frames);

		if (m_outputAvailableCb.inputProc)
		{
			AudioTimeStamp time = *inInputTime;
			time.mSampleTime += offset;
			if (offset && desc.mSampleRate > 0 && timebase.numer)
				time.mHostTime += uint64_t(double(offset) / desc.mSampleRate * 1e9 * timebase.denom / timebase.numer);

			// The client pulls the slice with AudioUnitRender(bus 1, frames) from this callback.
			AudioUnitRenderActionFlags flags = 0;
			m_outputAvailableCb.inputProc(m_outputAvailableCb.inputProcRefCon, &flags, &time, kInputBus, frames, nullptr);
		}
		offset += frames;
	}

	return noErr;
}

// AUDispatch.cpp doesn't implement dispatch code for AudioOutputUnits
OSStatus AUHAL::ComponentEntryDispatch(ComponentParameters *params, AUHAL *This)
{
	if (This == NULL) return kAudio_ParamError;

	OSStatus result = noErr;

	switch (params->what)
	{
		case kComponentCanDoSelect:
			switch (GetSelectorForCanDo(params))
			{
				case kAudioOutputUnitStartSelect:
				case kAudioOutputUnitStopSelect:
					return 1;
			}
			break;
		case kAudioOutputUnitStartSelect:
		{
			CAMutex::Locker lock(This->GetMutex());
			return This->Start();
		}
		case kAudioOutputUnitStopSelect:
		{
			CAMutex::Locker lock(This->GetMutex());
			return This->Stop();
		}
	}

	return AUBase::ComponentEntryDispatch(params, This);
}
