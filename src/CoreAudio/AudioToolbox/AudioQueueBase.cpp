#include "AudioQueueBase.h"
#include "stub.h"
#include <CarbonCore/MacErrors.h>

AudioQueue::AudioQueue(const AudioStreamBasicDescription* format, void* userData,
			CFRunLoopRef runloop, CFStringRef runloopMode, UInt32 flags)
: m_format(*format), m_userData(userData), m_flags(flags)
{
	// Both are optional: NULL means callbacks on the queue's own thread.
	m_runloop = runloop ? (CFRunLoopRef) CFRetain(runloop) : nullptr;
	m_runloopMode = runloopMode ? (CFStringRef) CFRetain(runloopMode) : nullptr;
}

AudioQueue::~AudioQueue()
{
	if (m_runloop)
		CFRelease(m_runloop);
	if (m_runloopMode)
		CFRelease(m_runloopMode);
}

OSStatus AudioQueue::getParameter(AudioQueueParameterID inParamID, AudioQueueParameterValue *outValue)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueue::setParameter(AudioQueueParameterID inParamID, AudioQueueParameterValue inValue)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueue::getProperty(AudioQueuePropertyID inID, void *outData, UInt32 *ioDataSize)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueue::setProperty(AudioQueuePropertyID inID, const void *inData, UInt32 inDataSize)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueue::getPropertySize(AudioQueuePropertyID inID, UInt32 *outDataSize)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueue::addPropertyListener(AudioQueuePropertyID inID, AudioQueuePropertyListenerProc inProc, void *inUserData)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueue::removePropertyListener(AudioQueuePropertyID inID, AudioQueuePropertyListenerProc inProc, void *inUserData)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueue::allocateBuffer(UInt32 inBufferByteSize, UInt32 inNumberPacketDescriptions, AudioQueueBufferRef *outBuffer)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueue::freeBuffer(AudioQueueBufferRef inBuffer)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueue::enqueueBuffer(AudioQueueBufferRef inBuffer, UInt32 inNumPacketDescs, const AudioStreamPacketDescription *inPacketDescs)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueue::getCurrentTime(AudioTimeStamp *outTimeStamp, Boolean *outTimelineDiscontinuity)
{
	STUB();
	return unimpErr;
}
