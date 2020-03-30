#include "AudioQueueBase.h"
#include "stub.h"
#include <CoreServices/MacErrors.h>

AudioQueue::AudioQueue(const AudioStreamBasicDescription* format, void* userData,
			CFRunLoopRef runloop, CFStringRef runloopMode, UInt32 flags)
: m_format(*format), m_userData(userData), m_flags(flags)
{
	m_runloop = (CFRunLoopRef) CFRetain(runloop);
	m_runloopMode = (CFStringRef) CFRetain(runloopMode);
}

AudioQueue::~AudioQueue()
{
	CFRelease(m_runloop);
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
