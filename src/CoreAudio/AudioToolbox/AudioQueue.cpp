#include "AudioQueue.h"
#include "AudioQueueOutput.h"
#include "AudioQueueInput.h"
#include <CarbonCore/MacErrors.h>

OSStatus AudioQueueStart(AudioQueueRef inAQ, const AudioTimeStamp *inStartTime)
{
	return inAQ->start(inStartTime);
}

OSStatus AudioQueuePrime(AudioQueueRef inAQ, UInt32 inNumberOfFramesToPrepare, UInt32 *outNumberOfFramesPrepared)
{
	return inAQ->prime(inNumberOfFramesToPrepare, outNumberOfFramesPrepared);
}

OSStatus AudioQueueFlush(AudioQueueRef inAQ)
{
	return inAQ->flush();
}

OSStatus AudioQueueStop(AudioQueueRef inAQ, Boolean inImmediate)
{
	return inAQ->stop(inImmediate);
}

OSStatus AudioQueuePause(AudioQueueRef inAQ)
{
	return inAQ->pause();
}

OSStatus AudioQueueReset(AudioQueueRef inAQ)
{
	return inAQ->reset();
}

OSStatus AudioQueueNewOutput(const AudioStreamBasicDescription *inFormat,
		AudioQueueOutputCallback inCallbackProc,
		void *inUserData, CFRunLoopRef inCallbackRunLoop,
		CFStringRef inCallbackRunLoopMode, UInt32 inFlags,
		AudioQueueRef *outAQ)
{
	return AudioQueueOutput::create(inFormat, inCallbackProc, inUserData,
			inCallbackRunLoop, inCallbackRunLoopMode, inFlags,
			(AudioQueueOutput**) outAQ);
}

OSStatus AudioQueueNewInput(const AudioStreamBasicDescription *inFormat,
		AudioQueueInputCallback inCallbackProc,
		void *inUserData, CFRunLoopRef inCallbackRunLoop,
		CFStringRef inCallbackRunLoopMode, UInt32 inFlags,
		AudioQueueRef *outAQ)
{
	return AudioQueueInput::create(inFormat, inCallbackProc, inUserData, inCallbackRunLoop,
			inCallbackRunLoopMode, nullptr, nullptr, (AudioQueueInput**) outAQ);
}

OSStatus AudioQueueNewInputWithDispatchQueue(AudioQueueRef *outAQ,
		const AudioStreamBasicDescription *inFormat, UInt32 inFlags,
		dispatch_queue_t inCallbackDispatchQueue,
		AudioQueueInputCallbackBlock inCallbackBlock)
{
	if (!inCallbackDispatchQueue || !inCallbackBlock)
		return kAudio_ParamError;
	return AudioQueueInput::create(inFormat, nullptr, nullptr, nullptr, nullptr,
			inCallbackDispatchQueue, inCallbackBlock, (AudioQueueInput**) outAQ);
}

OSStatus AudioQueueDispose(AudioQueueRef inAQ, Boolean inImmediate)
{
	return inAQ->dispose(inImmediate);
}

OSStatus AudioQueueGetParameter(AudioQueueRef inAQ, AudioQueueParameterID inParamID, AudioQueueParameterValue *outValue)
{
	return inAQ->getParameter(inParamID, outValue);
}

OSStatus AudioQueueSetParameter(AudioQueueRef inAQ, AudioQueueParameterID inParamID, AudioQueueParameterValue inValue)
{
	return inAQ->setParameter(inParamID, inValue);
}
 
OSStatus AudioQueueGetProperty(AudioQueueRef inAQ, AudioQueuePropertyID inID, void *outData, UInt32 *ioDataSize)
{
	return inAQ->getProperty(inID, outData, ioDataSize);
}

OSStatus AudioQueueSetProperty(AudioQueueRef inAQ, AudioQueuePropertyID inID, const void *inData, UInt32 inDataSize)
{
	return inAQ->setProperty(inID, inData, inDataSize);
}

OSStatus AudioQueueGetPropertySize(AudioQueueRef inAQ, AudioQueuePropertyID inID, UInt32 *outDataSize)
{
	return inAQ->getPropertySize(inID, outDataSize);
}

OSStatus AudioQueueAddPropertyListener(AudioQueueRef inAQ, AudioQueuePropertyID inID, AudioQueuePropertyListenerProc inProc, void *inUserData)
{
	if (!inAQ)
		return paramErr;
	return inAQ->addPropertyListener(inID, inProc, inUserData);
}

OSStatus AudioQueueRemovePropertyListener(AudioQueueRef inAQ, AudioQueuePropertyID inID, AudioQueuePropertyListenerProc inProc, void *inUserData)
{
	if (!inAQ)
		return paramErr;
	return inAQ->removePropertyListener(inID, inProc, inUserData);
}

OSStatus AudioQueueSetOfflineRenderFormat(AudioQueueRef inAQ, const AudioStreamBasicDescription *inFormat, const AudioChannelLayout *inLayout)
{
	return inAQ->setOfflineRenderFormat(inFormat, inLayout);
}

OSStatus AudioQueueOfflineRender(AudioQueueRef inAQ, const AudioTimeStamp *inTimestamp, AudioQueueBufferRef ioBuffer, UInt32 inNumberFrames)
{
	return inAQ->offlineRender(inTimestamp, ioBuffer, inNumberFrames);
}

OSStatus AudioQueueAllocateBuffer(AudioQueueRef inAQ, UInt32 inBufferByteSize, AudioQueueBufferRef *outBuffer)
{
	return inAQ->allocateBuffer(inBufferByteSize, 0, outBuffer);
}

OSStatus AudioQueueAllocateBufferWithPacketDescriptions(AudioQueueRef inAQ,
		UInt32 inBufferByteSize, UInt32 inNumberPacketDescriptions,
		AudioQueueBufferRef *outBuffer)
{
	return inAQ->allocateBuffer(inBufferByteSize, inNumberPacketDescriptions, outBuffer);
}

OSStatus AudioQueueFreeBuffer(AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
{
	return inAQ->freeBuffer(inBuffer);
}

OSStatus AudioQueueEnqueueBuffer(AudioQueueRef inAQ, AudioQueueBufferRef inBuffer,
		UInt32 inNumPacketDescs, const AudioStreamPacketDescription *inPacketDescs)
{
	return inAQ->enqueueBuffer(inBuffer, inNumPacketDescs, inPacketDescs);
}

OSStatus AudioQueueEnqueueBufferWithParameters(AudioQueueRef inAQ,
		AudioQueueBufferRef inBuffer, UInt32 inNumPacketDescs,
		const AudioStreamPacketDescription *inPacketDescs,
		UInt32 inTrimFramesAtStart, UInt32 inTrimFramesAtEnd,
		UInt32 inNumParamValues, const AudioQueueParameterEvent *inParamValues,
		const AudioTimeStamp *inStartTime, AudioTimeStamp *outActualStartTime)
{
	return inAQ->enqueueBufferWithParameters(inBuffer, inNumPacketDescs, inPacketDescs, inTrimFramesAtStart,
			inTrimFramesAtEnd, inNumParamValues, inParamValues, inStartTime, outActualStartTime);
}

// A timeline lets a client learn whether the queue's sample time was interrupted, as
// when its output device changes. The queue always plays to the default output unit,
// whose timeline runs on, so a timeline never reports a discontinuity.
struct OpaqueAudioQueueTimeline
{
	AudioQueueRef queue;
};

OSStatus AudioQueueCreateTimeline(AudioQueueRef inAQ, AudioQueueTimelineRef *outTimeline)
{
	if (!inAQ || !outTimeline)
		return kAudio_ParamError;
	*outTimeline = new OpaqueAudioQueueTimeline { inAQ };
	return noErr;
}

OSStatus AudioQueueDisposeTimeline(AudioQueueRef inAQ, AudioQueueTimelineRef inTimeline)
{
	if (!inAQ || !inTimeline || inTimeline->queue != inAQ)
		return kAudio_ParamError;
	delete inTimeline;
	return noErr;
}

OSStatus AudioQueueGetCurrentTime(AudioQueueRef inAQ, AudioQueueTimelineRef inTimeline, AudioTimeStamp *outTimeStamp, Boolean *outTimelineDiscontinuity)
{
	if (!inAQ)
		return kAudio_ParamError;
	if (inTimeline && inTimeline->queue != inAQ)
		return kAudio_ParamError;
	return inAQ->getCurrentTime(outTimeStamp, outTimelineDiscontinuity);
}

OSStatus AudioQueueProcessingTapNew(AudioQueueRef inAQ, AudioQueueProcessingTapCallback inCallback,
		void* inClientData, AudioQueueProcessingTapFlags inFlags, UInt32* outMaxFrames,
		AudioStreamBasicDescription* outProcessingFormat, AudioQueueProcessingTapRef* outAQTap)
{
	if (!inAQ || !inCallback || !outMaxFrames || !outProcessingFormat || !outAQTap)
		return kAudioQueueErr_InvalidParameter;
	return inAQ->processingTapNew(inCallback, inClientData, inFlags, outMaxFrames, outProcessingFormat, outAQTap);
}

OSStatus AudioQueueProcessingTapDispose(AudioQueueProcessingTapRef inAQTap)
{
	if (!inAQTap)
		return kAudioQueueErr_InvalidParameter;
	return inAQTap->queue->processingTapDispose(inAQTap);
}

OSStatus AudioQueueProcessingTapGetSourceAudio(AudioQueueProcessingTapRef inAQTap, UInt32 inNumberFrames,
		AudioTimeStamp* ioTimeStamp, AudioQueueProcessingTapFlags* outFlags, UInt32* outNumberFrames,
		AudioBufferList* ioData)
{
	if (!inAQTap)
		return kAudioQueueErr_InvalidParameter;
	return inAQTap->queue->processingTapGetSourceAudio(inAQTap, inNumberFrames, ioTimeStamp, outFlags, outNumberFrames, ioData);
}

OSStatus AudioQueueProcessingTapGetQueueTime(AudioQueueProcessingTapRef inAQTap,
		Float64* outQueueSampleTime, UInt32* outQueueFrameCount)
{
	if (!inAQTap)
		return kAudioQueueErr_InvalidParameter;
	return inAQTap->queue->processingTapGetQueueTime(inAQTap, outQueueSampleTime, outQueueFrameCount);
}
