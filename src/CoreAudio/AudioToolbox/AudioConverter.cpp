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

#include "AudioConverter.h"
#include "AudioConverterImpl.h"
#include <CoreServices/MacErrors.h>
#include <stdexcept>
#include <cstring>

OSStatus AudioConverterNew(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat, AudioConverterRef* outAudioConverter)
{
	return AudioConverter::create(inSourceFormat, inDestinationFormat, (AudioConverter**)outAudioConverter);
}

OSStatus AudioConverterNewSpecific(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat, UInt32 inNumberClassDescriptions, AudioClassDescription* nClassDescriptions, AudioConverterRef* outAudioConverter)
{
	// FIXME
	return AudioConverterNew(inSourceFormat, inDestinationFormat, outAudioConverter);
}

OSStatus AudioConverterReset(AudioConverterRef inAudioConverter)
{
	if (inAudioConverter)
	{
		((AudioConverter*)inAudioConverter)->flush();
		return noErr;
	}
	else
		return paramErr;
}

OSStatus AudioConverterDispose(AudioConverterRef inAudioConverter)
{
	delete (AudioConverter*)inAudioConverter;
	return noErr;
}

OSStatus AudioConverterGetProperty(AudioConverterRef inAudioConverter, AudioConverterPropertyID inPropertyID, UInt32 *ioPropertyDataSize, void *outPropertyData)
{
	return ((AudioConverter*)inAudioConverter)->getProperty(inPropertyID, ioPropertyDataSize, outPropertyData);
}

OSStatus AudioConverterGetPropertyInfo(AudioConverterRef inAudioConverter, AudioConverterPropertyID inPropertyID, UInt32 *outSize, Boolean *outWritable)
{
	return ((AudioConverter*)inAudioConverter)->getPropertyInfo(inPropertyID, outSize, outWritable);
}

OSStatus AudioConverterSetProperty(AudioConverterRef inAudioConverter, AudioConverterPropertyID inPropertyID, UInt32 inPropertyDataSize, const void *inPropertyData)
{
	return ((AudioConverter*)inAudioConverter)->setProperty(inPropertyID, inPropertyDataSize, inPropertyData);
}

OSStatus AudioConverterConvertBuffer(AudioConverterRef inAudioConverter, UInt32 inInputDataSize, const void* inInputData, UInt32 *ioOutputDataSize, void *outOutputData)
{
	AudioBufferList in, out;
	OSStatus stat;

	in.mNumberBuffers = out.mNumberBuffers = 1;

	in.mBuffers[0].mNumberChannels = 2;
	out.mBuffers[0].mNumberChannels = 2;

	in.mBuffers[0].mDataByteSize = inInputDataSize;
	in.mBuffers[0].mData = const_cast<void*>(inInputData);

	out.mBuffers[0].mDataByteSize = *ioOutputDataSize;
	out.mBuffers[0].mData = outOutputData;

	stat = AudioConverterConvertComplexBuffer(inAudioConverter, inInputDataSize/((AudioConverter*)inAudioConverter)->frameSize(), &in, &out);
	*ioOutputDataSize = out.mBuffers[0].mDataByteSize;

	return stat;
}

OSStatus AudioConverterFillComplexBuffer(AudioConverterRef inAudioConverter, AudioConverterComplexInputDataProc inInputDataProc, void *inInputDataProcUserData, UInt32 *ioOutputDataPacketSize, AudioBufferList *outOutputData, AudioStreamPacketDescription* outPacketDescription)
{
	return ((AudioConverter*)inAudioConverter)->fillComplex(inInputDataProc, inInputDataProcUserData, ioOutputDataPacketSize, outOutputData, outPacketDescription);
}

OSStatus AudioConverterConvertComplexBuffer(AudioConverterRef inAudioConverter, UInt32 inNumberPCMFrames, const AudioBufferList *inInputData, AudioBufferList *outOutputData)
{
	OSStatus status;
	UInt32 dataPacketSize;
	int64_t totalOutBytes = 0;
	typedef OSStatus (^CallbackBlock)(AudioConverterRef audioConverter, UInt32* numberDataPackets, AudioBufferList* data, AudioStreamPacketDescription** dataPacketDescription);
	
	CallbackBlock callbackBlock =
		^ OSStatus (AudioConverterRef audioConverter, UInt32* numberDataPackets, AudioBufferList* data, AudioStreamPacketDescription** dataPacketDescription)
	{
		const AudioBufferList *inInputData = (AudioBufferList*) inInputData;

		if (dataPacketDescription)
			*dataPacketDescription = nullptr;
		
		// TODO

		return unimpErr;
	};

	// Squeezing a block through a function pointer
	AudioConverterComplexInputDataProc proc = [](AudioConverterRef audioConverter, UInt32* numberDataPackets, AudioBufferList* data, AudioStreamPacketDescription** dataPacketDescription, void* userData) -> OSStatus
	{
		CallbackBlock callbackBlock = (CallbackBlock) userData;
		return callbackBlock(audioConverter, numberDataPackets, data, dataPacketDescription);
	};

	for (UInt32 i = 0; i < outOutputData->mNumberBuffers; i++)
		totalOutBytes += outOutputData->mBuffers[i].mDataByteSize;

	dataPacketSize = totalOutBytes / ((AudioConverter*)inAudioConverter)->frameSize();

	status = AudioConverterFillComplexBuffer(inAudioConverter, proc, (void*) inInputData, &dataPacketSize, outOutputData, nullptr);
	return status;
}
