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

#include "AudioHardwareImplPAOutput.h"
#include "AudioHardwareStreamPAOutput.h"

AudioHardwareImplPAOutput::AudioHardwareImplPAOutput(AudioObjectID myId, const char* paRole)
: AudioHardwareImplPA(myId, paRole)
{
	m_uid = CFSTR("PulseAudio:Output");
}

OSStatus AudioHardwareImplPAOutput::getPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32* ioDataSize, void* outData)
{
	if (inAddress->mSelector == kAudioDevicePropertyStreamConfiguration)
	{
		size_t size = sizeof(AudioBufferList);

		if (inAddress->mScope == kAudioDevicePropertyScopeOutput)
			size += 1* sizeof(AudioBuffer);

		// Number of returned buffers is the number of channels
		if (AudioBufferList* abl = static_cast<AudioBufferList*>(outData); abl && *ioDataSize >= size)
		{
			if (inAddress->mScope == kAudioDevicePropertyScopeOutput)
			{
				abl->mNumberBuffers = 1;
				abl->mBuffers[0].mNumberChannels = 2;

				// TODO: Is the below stuff ever used? How?
				abl->mBuffers[0].mData = nullptr;
				abl->mBuffers[0].mDataByteSize = 0;
			}
			else
				abl->mNumberBuffers = 0;
		}

		*ioDataSize = size;
		return kAudioHardwareNoError;
	}

	return AudioHardwareImplPA::getPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
}

OSStatus AudioHardwareImplPAOutput::setPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
	return AudioHardwareImplPA::setPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
}

AudioHardwareStream* AudioHardwareImplPAOutput::createStream(AudioDeviceIOProc callback, void* clientData)
{
	return new AudioHardwareStreamPAOutput(this, callback, clientData);
}
