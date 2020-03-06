/*
This file is part of Darling.

Copyright (C) 2015-2020 Lubos Dolezel

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

#include "AudioHardwareImpl.h"
#include <typeinfo>
#include <iostream>
#include <cstring>
#include <CoreServices/MacErrors.h>

AudioHardwareImpl::AudioHardwareImpl(AudioObjectID myId)
: m_myId(myId)
{
}


AudioHardwareImpl::~AudioHardwareImpl()
{
}
	
void AudioHardwareImpl::show()
{
	std::cout << typeid(*this).name() << std::endl;
}

OSStatus AudioHardwareImpl::createIOProcID(AudioDeviceIOProc inProc, void* inClientData,
		AudioDeviceIOProcID* outIOProcID)
{
	std::lock_guard<std::mutex> guard(m_procMutex);
	
	if (!inProc)
		return paramErr;
	
	AudioDeviceIOProcID procId;
	if (outIOProcID)
		*outIOProcID = procId = reinterpret_cast<AudioDeviceIOProcID>(m_nextProcId++);
	else
		procId = inProc;

	m_proc[procId] = std::make_pair(inProc, inClientData);
	
	return noErr;
}

OSStatus AudioHardwareImpl::destroyIOProcID(AudioDeviceIOProcID inIOProcID)
{
	std::lock_guard<std::mutex> guard(m_procMutex);
	auto it = m_proc.find(inIOProcID);
	
	if (it == m_proc.end())
		return paramErr;
	
	m_proc.erase(it);
	return noErr;
}

OSStatus AudioHardwareImpl::start(AudioDeviceIOProcID inProcID,
		AudioTimeStamp* ioRequestedStartTime, UInt32 inFlags)
{
	AudioHardwareStream* stream;
	std::lock_guard<std::mutex> guard(m_procMutex);
	
	if (m_streams.find(inProcID) != m_streams.end())
		return paramErr;

	auto it = m_proc.find(inProcID);
	
	if (it == m_proc.end())
		return paramErr;
	
	stream = createStream(it->second.first, it->second.second);
	if (!stream)
		return kAudioHardwareBadStreamError;
	m_streams.emplace(std::make_pair(inProcID, std::unique_ptr<AudioHardwareStream>(stream)));
	
	// TODO: time
	
	return noErr;
}

OSStatus AudioHardwareImpl::stop(AudioDeviceIOProcID inProcID)
{
	auto it = m_streams.find(inProcID);
	if (it == m_streams.end())
		return kAudioHardwareNotRunningError;
	
	AudioHardwareStream* stream = it->second.release();
	stream->stop(^{
		delete stream;
	});
	m_streams.erase(it);
	return noErr;
}

bool AudioHardwareImpl::hasProperty(const AudioObjectPropertyAddress* address)
{
	UInt32 ds;
	OSStatus status = getPropertyDataSize(address, 0, nullptr, &ds);
	return status != kAudioHardwareUnknownPropertyError;
}

OSStatus AudioHardwareImpl::getPropertyDataSize(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32* outDataSize)
{
	return getPropertyData(inAddress, inQualifierDataSize, inQualifierData, outDataSize, nullptr);
}

OSStatus AudioHardwareImpl::isPropertySettable(const AudioObjectPropertyAddress* inAddress, Boolean* outIsSettable)
{
	switch (inAddress->mSelector)
	{
		case kAudioDevicePropertyVolumeScalar:
		case kAudioDevicePropertyVolumeDecibels:
			*outIsSettable = true;
			return kAudioHardwareNoError;
	}

	*outIsSettable = false;
	return kAudioHardwareUnknownPropertyError;
}

// Examples:
// https://gist.github.com/robotconscience/490ee50d8ad75b47ea1f
// https://gist.github.com/hpux735/2913436

OSStatus AudioHardwareImpl::getPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32* ioDataSize, void* outData)
{
	switch (inAddress->mSelector)
	{
		case kAudioDevicePropertyStreamFormat: // returns AudioStreamBasicDescription
		{
			if (AudioStreamBasicDescription* asbd = static_cast<AudioStreamBasicDescription*>(outData); asbd && *ioDataSize >= sizeof(AudioStreamBasicDescription))
			{
				// These are standard values macOS applications have learned to expect
				asbd->mChannelsPerFrame = 2;
				asbd->mSampleRate = 44100;
				asbd->mBitsPerChannel = 32;
				asbd->mFormatID = kAudioFormatLinearPCM;
				asbd->mFormatFlags = kLinearPCMFormatFlagIsFloat;
				asbd->mFramesPerPacket = 1;
				asbd->mBytesPerFrame = UInt32(asbd->mBytesPerPacket = asbd->mChannelsPerFrame * asbd->mSampleRate * asbd->mBitsPerChannel / 8);
			}
			*ioDataSize = sizeof(AudioStreamBasicDescription);
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyDeviceManufacturer:
			return getPropertyString(m_manufacturer, ioDataSize, outData);
		case kAudioDevicePropertyDeviceManufacturerCFString:
			return getPropertyCFString(m_manufacturer, ioDataSize, outData);
		// NOTE: This is related to [NSSound setPlaybackDeviceIdentifier]
		case kAudioDevicePropertyDeviceUID: // returns CFStringRef
			return getPropertyCFString(m_uid, ioDataSize, outData);
		case kAudioDevicePropertyModelUID:
		return getPropertyCFString(m_modelUid, ioDataSize, outData);
		case kAudioDevicePropertyDeviceNameCFString:
			return getPropertyCFString(m_name, ioDataSize, outData);
		case kAudioDevicePropertyDeviceName: // return char[]
			return getPropertyString(m_name, ioDataSize, outData);
		case kAudioDevicePropertyNominalSampleRate: // Float64
		{
			if (Float64* flt = static_cast<Float64*>(outData); flt && *ioDataSize >= sizeof(Float64))
				*flt = 44100;

			*ioDataSize = sizeof(Float64);
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyAvailableNominalSampleRates: // AudioValueRange[]
		{
			if (AudioValueRange* avr = static_cast<AudioValueRange*>(outData); avr && *ioDataSize >= sizeof(AudioValueRange)*1)
			{
				avr->mMinimum = 44100;
				avr->mMaximum = 44100;
			}
			*ioDataSize = sizeof(AudioValueRange) * 1;
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyDeviceIsAlive: // int
		case kAudioDevicePropertyDeviceIsRunning:
		{
			if (int* b = static_cast<int*>(outData); outData && *ioDataSize >= sizeof(int))
				*b = 1;
			*ioDataSize = sizeof(int);
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyIsHidden:
		{
			if (int* b = static_cast<int*>(outData); outData && *ioDataSize >= sizeof(int))
				*b = 0;
			*ioDataSize = sizeof(int);
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyPreferredChannelsForStereo:
		{
			if (UInt32* b = static_cast<UInt32*>(outData); outData && *ioDataSize >= sizeof(UInt32)*2)
			{
				b[0] = 0;
				b[1] = 1;
			}
			*ioDataSize = sizeof(UInt32)*2;
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyVolumeScalar:
		{
			if (Float32* v = static_cast<Float32*>(outData); outData && *ioDataSize >= sizeof(Float32))
				*v = 1.0f; // TODO: Ask for real volume from PulseAudio
			*ioDataSize = sizeof(Float32);
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyVolumeDecibels:
		{
			return kAudioHardwareNoError;
		}
	}
	return kAudioHardwareUnknownPropertyError;
}

OSStatus AudioHardwareImpl::getPropertyCFString(CFStringRef str, UInt32* ioDataSize, void* outData)
{
	if (CFStringRef* ref = static_cast<CFStringRef*>(outData); ref && *ioDataSize >= sizeof(CFStringRef))
	{
		*ref = (CFStringRef) CFRetain(str);
	}
	*ioDataSize = sizeof(CFStringRef);
	return noErr;
}

OSStatus AudioHardwareImpl::getPropertyString(CFStringRef str, UInt32* ioDataSize, void* outData)
{
	const char* cstr = CFStringGetCStringPtr(str, kCFStringEncodingUTF8);
	if (char* target = static_cast<char*>(outData))
		strlcpy(target, cstr, *ioDataSize);
	*ioDataSize = strlen(cstr) + 1;
	return noErr;
}

OSStatus AudioHardwareImpl::setPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
	switch (inAddress->mSelector)
	{
		case kAudioDevicePropertyBufferSize:
		{
			return setBufferSize(*static_cast<const uint32_t*>(inData));
		}
		case kAudioDevicePropertyVolumeScalar:
		{
			// TODO: Implement setting the volume
			// Note: qualifier contains the channel ID (from kAudioDevicePropertyPreferredChannelsForStereo)
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyVolumeDecibels:
		{
			return kAudioHardwareNoError;
		}
		// These make sense only for ALSA, but I find it ridiculous that these properties can be set...
		case kAudioHardwarePropertyDefaultOutputDevice:
			return kAudioHardwareNoError;
		case kAudioHardwarePropertyDefaultInputDevice:
			return kAudioHardwareNoError;
	}
	return kAudioHardwareUnknownPropertyError;
}

OSStatus AudioHardwareImpl::addPropertyListener(const AudioObjectPropertyAddress* inAddress,
	AudioObjectPropertyListenerProc inListener, void* inClientData)
{
	return kAudioHardwareUnknownPropertyError;
}

OSStatus AudioHardwareImpl::removePropertyListener(const AudioObjectPropertyAddress* inAddress,
	AudioObjectPropertyListenerProc inListener, void* inClientData)
{
	return kAudioHardwareUnknownPropertyError;
}

OSStatus AudioHardwareImpl::setBufferSize(uint32_t bufferSize)
{
	m_bufferSize = bufferSize;
	return noErr;
}

OSStatus AudioHardwareImpl::getCurrentTime(AudioTimeStamp* outTime)
{
	std::memset(outTime, 0, sizeof(*outTime));
	return noErr;
}
	
OSStatus AudioHardwareImpl::translateTime(const AudioTimeStamp* inTime, AudioTimeStamp* outTime)
{
	*outTime = *inTime;
	return noErr;
}

OSStatus AudioHardwareImpl::getNearestStartTime(AudioTimeStamp* ioRequestedStartTime, UInt32 inFlags)
{
	std::memset(ioRequestedStartTime, 0, sizeof(*ioRequestedStartTime));
	return noErr;
}
