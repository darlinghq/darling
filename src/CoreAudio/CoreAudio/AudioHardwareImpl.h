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
#include <map>
#include <mutex>
#include "AudioHardwareStream.h"

class AudioHardwareImpl
{
public:
	AudioHardwareImpl();
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
	
	virtual OSStatus getCurrentTime(AudioTimeStamp* outTime) = 0;
	
	virtual OSStatus translateTime(const AudioTimeStamp* inTime,
		AudioTimeStamp* outTime) = 0;
	
	virtual OSStatus getNearestStartTime(AudioTimeStamp* ioRequestedStartTime,
		UInt32 inFlags) = 0;
protected:
	virtual AudioHardwareStream* createStream(AudioDeviceIOProcID procID) = 0;
protected:
	std::mutex m_procMutex;
	std::map<AudioDeviceIOProcID, std::pair<AudioDeviceIOProc, void*>> m_proc;
	int m_nextProcId = 1;
	
	std::map<AudioDeviceIOProcID, std::unique_ptr<AudioHardwareStream> m_streams;
};

#endif /* AUDIOHARDWAREIMPL_H */

