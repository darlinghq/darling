#include "AudioQueue.h"
#include "AudioQueueOutput.h"
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
	*outAQ = nullptr;
	return unimpErr;
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
	return inAQ->addPropertyListener(inID, inProc, inUserData);
}

OSStatus AudioQueueRemovePropertyListener(AudioQueueRef inAQ, AudioQueuePropertyID inID, AudioQueuePropertyListenerProc inProc, void *inUserData)
{
	return inAQ->addPropertyListener(inID, inProc, inUserData);
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
	// Trimming, scheduled start times and per-buffer parameters are not supported:
	// accept the plain case only, rather than play something else than asked.
	if (inTrimFramesAtStart || inTrimFramesAtEnd || inNumParamValues || inStartTime || outActualStartTime)
		return unimpErr;
	return inAQ->enqueueBuffer(inBuffer, inNumPacketDescs, inPacketDescs);
}

OSStatus AudioQueueGetCurrentTime(AudioQueueRef inAQ, AudioQueueTimelineRef inTimeline, AudioTimeStamp *outTimeStamp, Boolean *outTimelineDiscontinuity)
{
	return inAQ->getCurrentTime(outTimeStamp, outTimelineDiscontinuity);
}
