/*
This file is part of Darling.

Copyright (C) 2015-2016 Lubos Dolezel

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

#include "AudioHardware.h"
#include "AudioHardwareImpl.h"
#include <CoreServices/MacErrors.h>
#include <memory>

static std::unique_ptr<AudioHardwareImpl> g_systemObject;

static AudioHardwareImpl* GetSystemObject()
{
	if (!g_systemObject)
	{
		static std::mutex singleLock;
		singleLock.lock();
		
		if (!g_systemObject)
			g_systemObject.reset(nullptr); // TODO: create ALSA or PA here
		
		singleLock.unlock();
	}
	
	return g_systemObject.get();
}

static AudioHardwareImpl* GetObject(AudioObjectID objID)
{
	if (objID == kAudioObjectSystemObject)
		return GetSystemObject();
	
	return nullptr;
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
		return kAudioHardwareBadObjectError;
	
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
	g_systemObject.reset(nullptr);
	return noErr;
}

OSStatus AudioHardwareCreateAggregateDevice(CFDictionaryRef, AudioObjectID* outDeviceID)
{
	if (outDeviceID)
		*outDeviceID = 0;
	
	return unimpErr;
}

OSStatus AudioHardwareDestroyAggregateDevice(AudioObjectID inDeviceID)
{
	return unimpErr;
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

OSStatus AudioDeviceDestroyIOProcID(AudioObjectID inDevice,
		AudioDeviceIOProcID inIOProcID)
{
	AudioHardwareImpl* obj = GetObject(inDevice);
	if (!obj)
		return kAudioHardwareBadObjectError;
	
	return obj->destroyIOProcID(inIOProcID);
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
