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

#include "AudioHardwareImplPA.h"

AudioHardwareImplPA::AudioHardwareImplPA()
{
	m_name = CFSTR("PulseAudio");
	m_manufacturer = CFSTR("PulseAudio");
	m_uid = CFSTR("100");
}

OSStatus AudioHardwareImplPA::getPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32* ioDataSize, void* outData)
{
	switch (inAddress->mSelector)
	{
		// BEGIN kAudioObjectSystemObject properties
		case kAudioHardwarePropertyDefaultInputDevice: // returns AudioDeviceID
			if (AudioDeviceID* devId = static_cast<AudioDeviceID*>(outData); devId && *ioDataSize >= sizeof(AudioDeviceID))
			{
				// We're doing a bit of an abuse here, which works only for PA where we have a single virtual device.
				// For ALSA, we could do kAudioObjectSystemObject + INDEX of device.
				*devId = kAudioObjectSystemObject;
			}
			*ioDataSize = sizeof(AudioDeviceID);
			break;
		case kAudioHardwarePropertyDefaultOutputDevice: // returns AudioDeviceID
			if (AudioDeviceID* devId = static_cast<AudioDeviceID*>(outData); devId && *ioDataSize >= sizeof(AudioDeviceID))
			{
				*devId = kAudioObjectSystemObject;
			}
			*ioDataSize = sizeof(AudioDeviceID);
			break;
		case kAudioHardwarePropertyDevices:
		{
			if (AudioDeviceID* devId = static_cast<AudioDeviceID*>(outData); devId && *ioDataSize >= sizeof(AudioDeviceID))
			{
				devId[0] = kAudioObjectSystemObject;
			}
			*ioDataSize = sizeof(AudioDeviceID) * 1;
			break;
		}
		// END kAudioObjectSystemObject properties

		// BEGIN properties of a specific audio device (in case of PA, it is kAudioObjectSystemObject as well)
		case kAudioDevicePropertyStreamConfiguration: // returns AudioBufferList
		{
			const size_t size = sizeof(AudioBufferList) + 1* sizeof(AudioBuffer);

			// Number of returned buffers is the number of channels
			// check inAddress->mScope. If it equals kAudioDevicePropertyScopeInput, the caller cares about input only.
			if (AudioBufferList* abl = static_cast<AudioBufferList*>(outData); abl && *ioDataSize >= size)
			{
				abl->mNumberBuffers = 1;
				abl->mBuffers[0].mNumberChannels = 2;

				// TODO: Is the below stuff ever used? How?
				abl->mBuffers[0].mData = nullptr;
				abl->mBuffers[0].mDataByteSize = 0;
			}

			*ioDataSize = size;
			break;
		}
		
	}

	return AudioHardwareImpl::getPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
}
	
OSStatus AudioHardwareImplPA::setPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
	return AudioHardwareImpl::setPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
}

AudioHardwareStream* AudioHardwareImplPA::createStream(AudioDeviceIOProcID procID)
{
	// TODO
	return nullptr;
}
