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

#include <CoreAudio/AudioHardware.h>
#include "AudioHardwareImpl.h"
#include "pulse/AudioHardwareImplPA.h"
#include "pulse/AudioHardwareImplPAOutput.h"
#include "pulse/AudioHardwareImplPAInput.h"
#include <CarbonCore/MacErrors.h>
#include <dispatch/dispatch.h>
#include <memory>
#include <unordered_map>
#include "stub.h"

static std::unordered_map<AudioObjectID, std::unique_ptr<AudioHardwareImpl>> g_objects;

static void initObjects()
{
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		// TODO: Or ALSA
		g_objects.insert(std::make_pair(kAudioObjectSystemObject, std::make_unique<AudioHardwareImplPA>(kAudioObjectSystemObject)));
		g_objects.insert(std::make_pair(kAudioObjectSystemObject + 1, std::make_unique<AudioHardwareImplPAOutput>(kAudioObjectSystemObject + 1)));
		g_objects.insert(std::make_pair(kAudioObjectSystemObject + 2, std::make_unique<AudioHardwareImplPAInput>(kAudioObjectSystemObject + 2)));
		g_objects.insert(std::make_pair(kAudioObjectSystemObject + 1, std::make_unique<AudioHardwareImplPAOutput>(kAudioObjectSystemObject + 3, "event")));
	});
}

static AudioHardwareImpl* GetObject(AudioObjectID objID)
{
	initObjects();

	auto it = g_objects.find(objID);
	if (it == g_objects.end())
		return nullptr;

	return it->second.get();
}

static AudioHardwareImpl* GetSystemObject()
{
	return GetObject(kAudioObjectSystemObject);
}

void AudioObjectShow(AudioObjectID inObjectID)
{
	AudioHardwareImpl* obj = GetObject(inObjectID);
	if (obj)
		obj->show();
}

Boolean AudioObjectHasProperty(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress)
{
	AudioHardwareImpl* obj = GetObject(inObjectID);
	if (!obj)
		return 0;
	
	return obj->hasProperty(inAddress);
}

OSStatus AudioObjectIsPropertySettable(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress, Boolean* outIsSettable)
{
	AudioHardwareImpl* obj = GetObject(inObjectID);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->isPropertySettable(inAddress, outIsSettable);
}

OSStatus AudioObjectGetPropertyDataSize(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32* outDataSize)
{
	AudioHardwareImpl* obj = GetObject(inObjectID);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->getPropertyDataSize(inAddress, inQualifierDataSize,
			inQualifierData, outDataSize);
}

OSStatus AudioObjectGetPropertyData(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32* ioDataSize, void* outData)
{
	AudioHardwareImpl* obj = GetObject(inObjectID);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->getPropertyData(inAddress, inQualifierDataSize, inQualifierData,
			ioDataSize, outData);
}

OSStatus AudioObjectSetPropertyData(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
	AudioHardwareImpl* obj = GetObject(inObjectID);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->setPropertyData(inAddress, inQualifierDataSize, inQualifierData,
			inDataSize, inData);
}

OSStatus AudioObjectAddPropertyListener(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress,
		AudioObjectPropertyListenerProc inListener, void* inClientData)
{
	AudioHardwareImpl* obj = GetObject(inObjectID);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->addPropertyListener(inAddress, inListener, inClientData);
}

OSStatus AudioObjectRemovePropertyListener(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress,
		AudioObjectPropertyListenerProc inListener, void* inClientData)
{
	AudioHardwareImpl* obj = GetObject(inObjectID);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->removePropertyListener(inAddress, inListener, inClientData);
}

OSStatus AudioHardwareUnload(void)
{
	return noErr;
}

OSStatus AudioHardwareCreateAggregateDevice(CFDictionaryRef, AudioObjectID* outDeviceID)
{
	STUB();
	if (outDeviceID)
		*outDeviceID = 0;
	
	return unimpErr;
}

OSStatus AudioHardwareDestroyAggregateDevice(AudioObjectID inDeviceID)
{
	STUB();
	return unimpErr;
}

OSStatus AudioHardwareGetProperty(AudioHardwarePropertyID inPropId, UInt32* ioPropertyDataSize, void* outPropertyData)
{
	if (!ioPropertyDataSize)
		return paramErr;

	return AudioDeviceGetProperty(kAudioObjectSystemObject, 0, false, inPropId, ioPropertyDataSize, outPropertyData);
}

OSStatus AudioDeviceGetProperty(AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput, AudioDevicePropertyID inPropertyID, UInt32* ioPropertyDataSize, void* outPropertyData)
{
	AudioObjectPropertyAddress aopa = {
		inPropertyID,
		isInput ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput, // kAudioObjectPropertyScopeGlobal
		kAudioObjectPropertyElementMaster
	};

	return AudioObjectGetPropertyData(inDevice, &aopa, 0, nullptr, ioPropertyDataSize, outPropertyData);
}

OSStatus AudioDeviceGetPropertyInfo(AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput, AudioDevicePropertyID inPropertyID, UInt32* outSize, Boolean* outWritable)
{
	AudioObjectPropertyAddress aopa = {
		inPropertyID,
		isInput ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput, // kAudioObjectPropertyScopeGlobal
		kAudioObjectPropertyElementMaster
	};

	OSStatus status;
	if (outSize)
	{
		status = AudioObjectGetPropertyDataSize(inDevice, &aopa, 0, nullptr, outSize);
		if (status != noErr)
			return status;
	}

	if (outWritable)
	{
		status = AudioObjectIsPropertySettable(inDevice, &aopa, outWritable);
	}
	return status;
}

OSStatus AudioDeviceSetProperty(AudioDeviceID inDevice, const AudioTimeStamp *inWhen, UInt32 inChannel,
	Boolean isInput, AudioDevicePropertyID inPropertyID, UInt32 inPropertyDataSize, const void *inPropertyData)
{
	AudioObjectPropertyAddress aopa = {
		inPropertyID,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster
	};

	return AudioObjectSetPropertyData(inDevice, &aopa, 0, nullptr, inPropertyDataSize, inPropertyData);
}

OSStatus AudioHardwareGetPropertyInfo(AudioHardwarePropertyID inPropertyID, UInt32 *outSize, Boolean *outWritable)
{
	return AudioDeviceGetPropertyInfo(kAudioObjectSystemObject, 0, false, inPropertyID, outSize, outWritable);
}

OSStatus AudioDeviceCreateIOProcID(AudioObjectID inDevice,
		AudioDeviceIOProc inProc, void* inClientData,
		AudioDeviceIOProcID* outIOProcID)
{
	AudioHardwareImpl* obj = GetObject(inDevice);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->createIOProcID(inProc, inClientData, outIOProcID);
}

OSStatus AudioDeviceAddIOProc(AudioDeviceID inDevice, AudioDeviceIOProc inProc, void *inClientData)
{
	AudioHardwareImpl* obj = GetObject(inDevice);
	if (!obj)
		return kAudioHardwareBadObjectError;
	return obj->createIOProcID(inProc, inClientData, nullptr);
}

OSStatus AudioDeviceDestroyIOProcID(AudioObjectID inDevice,
		AudioDeviceIOProcID inIOProcID)
{
	AudioHardwareImpl* obj = GetObject(inDevice);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->destroyIOProcID(inIOProcID);
}

OSStatus AudioDeviceRemoveIOProc(AudioDeviceID inDevice, AudioDeviceIOProc inProc)
{
	AudioHardwareImpl* obj = GetObject(inDevice);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->destroyIOProcID(AudioDeviceIOProcID(inProc));
}

OSStatus AudioDeviceStart(AudioObjectID inDevice, AudioDeviceIOProcID inProcID)
{
	AudioHardwareImpl* obj = GetObject(inDevice);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->start(inProcID, nullptr, 0);
}

OSStatus AudioDeviceStartAtTime(AudioObjectID inDevice, AudioDeviceIOProcID inProcID,
		AudioTimeStamp* ioRequestedStartTime, UInt32 inFlags)
{
	AudioHardwareImpl* obj = GetObject(inDevice);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->start(inProcID, ioRequestedStartTime, inFlags);
}

OSStatus AudioDeviceStop(AudioObjectID inDevice, AudioDeviceIOProcID inProcID)
{
	AudioHardwareImpl* obj = GetObject(inDevice);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->stop(inProcID);
}

OSStatus AudioDeviceGetCurrentTime(AudioObjectID inDevice, AudioTimeStamp* outTime)
{
	AudioHardwareImpl* obj = GetObject(inDevice);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->getCurrentTime(outTime);
}

OSStatus AudioDeviceTranslateTime(AudioObjectID inDevice, const AudioTimeStamp* inTime,
		AudioTimeStamp* outTime)
{
	AudioHardwareImpl* obj = GetObject(inDevice);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->translateTime(inTime, outTime);
}

OSStatus AudioDeviceGetNearestStartTime(AudioObjectID inDevice,
		AudioTimeStamp* ioRequestedStartTime, UInt32 inFlags)
{
	AudioHardwareImpl* obj = GetObject(inDevice);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->getNearestStartTime(ioRequestedStartTime, inFlags);
}
