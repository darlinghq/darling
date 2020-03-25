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

#ifndef AUDIOHARDWAREIMPL_H
#define AUDIOHARDWAREIMPL_H
#include <CoreAudio/AudioHardware.h>
#include <CoreFoundation/CFString.h>
#include <map>
#include <mutex>
#include "AudioHardwareStream.h"

class AudioHardwareImpl
{
public:
	AudioHardwareImpl(AudioObjectID myId);
	virtual ~AudioHardwareImpl();
	
	virtual void show();
	virtual bool hasProperty(const AudioObjectPropertyAddress* address);
	virtual OSStatus getPropertyDataSize(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32* outDataSize);
	
	virtual OSStatus isPropertySettable(const AudioObjectPropertyAddress* inAddress, Boolean* outIsSettable);
	
	virtual OSStatus getPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32* ioDataSize, void* outData);
	
	virtual OSStatus setPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32 inDataSize, const void* inData);
	
	virtual OSStatus addPropertyListener(const AudioObjectPropertyAddress* inAddress,
		AudioObjectPropertyListenerProc inListener, void* inClientData);
	
	virtual OSStatus removePropertyListener(const AudioObjectPropertyAddress* inAddress,
		AudioObjectPropertyListenerProc inListener, void* inClientData);
	
	OSStatus createIOProcID(AudioDeviceIOProc inProc, void* inClientData,
		AudioDeviceIOProcID* outIOProcID);
	OSStatus destroyIOProcID(AudioDeviceIOProcID inIOProcID);
	
	virtual OSStatus start(AudioDeviceIOProcID inProcID,
		AudioTimeStamp* ioRequestedStartTime, UInt32 inFlags);
	virtual OSStatus stop(AudioDeviceIOProcID inProcID);
	
	virtual OSStatus getCurrentTime(AudioTimeStamp* outTime);
	
	virtual OSStatus translateTime(const AudioTimeStamp* inTime,
		AudioTimeStamp* outTime);
	
	virtual OSStatus getNearestStartTime(AudioTimeStamp* ioRequestedStartTime,
		UInt32 inFlags);
	virtual OSStatus setBufferSize(uint32_t bufferSize);
	uint32_t bufferSize() const { return m_bufferSize; }

	AudioObjectID id() const { return m_myId; }
	const AudioStreamBasicDescription& asbd() const { return m_asbd; }
private:
	static OSStatus getPropertyCFString(CFStringRef str, UInt32* ioDataSize, void* outData);
	static OSStatus getPropertyString(CFStringRef str, UInt32* ioDataSize, void* outData);
protected:
	virtual AudioHardwareStream* createStream(AudioDeviceIOProc callback, void* clientData) = 0;
	virtual bool validateFormat(const AudioStreamBasicDescription* absd) const { return true; }
protected:
	AudioObjectID m_myId;
	std::mutex m_procMutex;
	std::map<AudioDeviceIOProcID, std::pair<AudioDeviceIOProc, void*>> m_proc;
	int m_nextProcId = 1;
	
	std::map<AudioDeviceIOProcID, std::unique_ptr<AudioHardwareStream>> m_streams;
	uint32_t m_bufferSize = 8192;

	CFStringRef m_name = CFSTR("unknown"), m_uid = CFSTR("unknown"), m_manufacturer = CFSTR("unknown"), m_modelUid = CFSTR("unknown");
	AudioStreamBasicDescription m_asbd;
};

#endif /* AUDIOHARDWAREIMPL_H */

