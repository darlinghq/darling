#include "AudioConverter.h"
#include "AudioConverterImpl.h"
#include <CoreServices/MacErrors.h>
#include <util/debug.h>
#include <stdexcept>
#include <cstring>

OSStatus AudioConverterNew(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat, AudioConverterRef* outAudioConverter)
{
	return AudioConverter::create(inSourceFormat, inDestinationFormat, outAudioConverter);
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
		inAudioConverter->flush();
		return noErr;
	}
	else
		return paramErr;
}

OSStatus AudioConverterDispose(AudioConverterRef inAudioConverter)
{
	delete inAudioConverter;
	return noErr;
}

OSStatus AudioConverterGetProperty(AudioConverterRef inAudioConverter, AudioConverterPropertyID inPropertyID, UInt32 *ioPropertyDataSize, void *outPropertyData)
{
	return inAudioConverter->getProperty(inPropertyID, ioPropertyDataSize, outPropertyData);
}

OSStatus AudioConverterGetPropertyInfo(AudioConverterRef inAudioConverter, AudioConverterPropertyID inPropertyID, UInt32 *outSize, Boolean *outWritable)
{
	return inAudioConverter->getPropertyInfo(inPropertyID, outSize, outWritable);
}

OSStatus AudioConverterSetProperty(AudioConverterRef inAudioConverter, AudioConverterPropertyID inPropertyID, UInt32 inPropertyDataSize, const void *inPropertyData)
{
	return inAudioConverter->setProperty(inPropertyID, inPropertyDataSize, inPropertyData);
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

	stat = AudioConverterConvertComplexBuffer(inAudioConverter, inInputDataSize/inAudioConverter->frameSize(), &in, &out);
	*ioOutputDataSize = out.mBuffers[0].mDataByteSize;

	return stat;
}

OSStatus AudioConverterFillComplexBuffer(AudioConverterRef inAudioConverter, AudioConverterComplexInputDataProc inInputDataProc, void *inInputDataProcUserData, UInt32 *ioOutputDataPacketSize, AudioBufferList *outOutputData, AudioStreamPacketDescription* outPacketDescription)
{
	return inAudioConverter->fillComplex(inInputDataProc, inInputDataProcUserData, ioOutputDataPacketSize, outOutputData, outPacketDescription);
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

	dataPacketSize = totalOutBytes / inAudioConverter->frameSize();

	status = AudioConverterFillComplexBuffer(inAudioConverter, proc, (void*) inInputData, &dataPacketSize, outOutputData, nullptr);
	return status;
}
