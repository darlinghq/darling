#include "AudioQueueBase.h"
#include "stub.h"
#include <CarbonCore/MacErrors.h>

OpaqueAudioQueue::OpaqueAudioQueue(const AudioStreamBasicDescription* format, void* userData,
			CFRunLoopRef runloop, CFStringRef runloopMode, UInt32 flags)
: m_format(*format), m_userData(userData), m_flags(flags)
{
	// Both are optional: NULL means callbacks on the queue's own thread.
	m_runloop = runloop ? (CFRunLoopRef) CFRetain(runloop) : nullptr;
	m_runloopMode = runloopMode ? (CFStringRef) CFRetain(runloopMode) : nullptr;
}

OpaqueAudioQueue::~OpaqueAudioQueue()
{
	if (m_runloop)
		CFRelease(m_runloop);
	if (m_runloopMode)
		CFRelease(m_runloopMode);
}

OSStatus OpaqueAudioQueue::getParameter(AudioQueueParameterID inParamID, AudioQueueParameterValue *outValue)
{
	STUB();
	return unimpErr;
}

OSStatus OpaqueAudioQueue::setParameter(AudioQueueParameterID inParamID, AudioQueueParameterValue inValue)
{
	STUB();
	return unimpErr;
}

OSStatus OpaqueAudioQueue::getProperty(AudioQueuePropertyID inID, void *outData, UInt32 *ioDataSize)
{
	STUB();
	return unimpErr;
}

OSStatus OpaqueAudioQueue::setProperty(AudioQueuePropertyID inID, const void *inData, UInt32 inDataSize)
{
	STUB();
	return unimpErr;
}

OSStatus OpaqueAudioQueue::getPropertySize(AudioQueuePropertyID inID, UInt32 *outDataSize)
{
	STUB();
	return unimpErr;
}

OSStatus OpaqueAudioQueue::addPropertyListener(AudioQueuePropertyID inID, AudioQueuePropertyListenerProc inProc, void *inUserData)
{
	if (!inProc)
		return paramErr;
	std::lock_guard<std::mutex> lock(m_listenerMutex);
	m_listeners.push_back({ inID, inProc, inUserData });
	return noErr;
}

OSStatus OpaqueAudioQueue::removePropertyListener(AudioQueuePropertyID inID, AudioQueuePropertyListenerProc inProc, void *inUserData)
{
	std::lock_guard<std::mutex> lock(m_listenerMutex);
	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		if (it->id == inID && it->proc == inProc && it->userData == inUserData)
		{
			m_listeners.erase(it);
			break;
		}
	}
	return noErr;
}

void OpaqueAudioQueue::notifyListeners(AudioQueuePropertyID id)
{
	std::vector<Listener> listeners;
	{
		std::lock_guard<std::mutex> lock(m_listenerMutex);
		for (const Listener& l : m_listeners)
		{
			if (l.id == id)
				listeners.push_back(l);
		}
	}
	if (listeners.empty())
		return;
	AudioQueueRef queue = this;
	runOnClient([listeners, queue, id] {
		for (const Listener& l : listeners)
			l.proc(l.userData, queue, id);
	});
}

OSStatus OpaqueAudioQueue::enqueueBufferWithParameters(AudioQueueBufferRef inBuffer, UInt32 inNumPacketDescs,
		const AudioStreamPacketDescription *inPacketDescs, UInt32 inTrimFramesAtStart, UInt32 inTrimFramesAtEnd,
		UInt32 inNumParamValues, const AudioQueueParameterEvent *inParamValues,
		const AudioTimeStamp *inStartTime, AudioTimeStamp *outActualStartTime)
{
	// Trimming, scheduled start times and per-buffer parameters are not supported by default:
	// accept the plain case only, rather than play something else than asked.
	if (inTrimFramesAtStart || inTrimFramesAtEnd || inNumParamValues || inStartTime || outActualStartTime)
		return unimpErr;
	return enqueueBuffer(inBuffer, inNumPacketDescs, inPacketDescs);
}

OSStatus OpaqueAudioQueue::allocateBuffer(UInt32 inBufferByteSize, UInt32 inNumberPacketDescriptions, AudioQueueBufferRef *outBuffer)
{
	STUB();
	return unimpErr;
}

OSStatus OpaqueAudioQueue::freeBuffer(AudioQueueBufferRef inBuffer)
{
	STUB();
	return unimpErr;
}

OSStatus OpaqueAudioQueue::enqueueBuffer(AudioQueueBufferRef inBuffer, UInt32 inNumPacketDescs, const AudioStreamPacketDescription *inPacketDescs)
{
	STUB();
	return unimpErr;
}

OSStatus OpaqueAudioQueue::getCurrentTime(AudioTimeStamp *outTimeStamp, Boolean *outTimelineDiscontinuity)
{
	STUB();
	return unimpErr;
}

OSStatus OpaqueAudioQueue::processingTapNew(AudioQueueProcessingTapCallback inCallback, void* inClientData,
		AudioQueueProcessingTapFlags inFlags, UInt32* outMaxFrames,
		AudioStreamBasicDescription* outProcessingFormat, AudioQueueProcessingTapRef* outAQTap)
{
	STUB();
	return unimpErr;
}

OSStatus OpaqueAudioQueue::processingTapDispose(AudioQueueProcessingTapRef inAQTap)
{
	STUB();
	return unimpErr;
}

OSStatus OpaqueAudioQueue::processingTapGetSourceAudio(AudioQueueProcessingTapRef inAQTap, UInt32 inNumberFrames,
		AudioTimeStamp* ioTimeStamp, AudioQueueProcessingTapFlags* outFlags, UInt32* outNumberFrames,
		AudioBufferList* ioData)
{
	STUB();
	return unimpErr;
}

OSStatus OpaqueAudioQueue::processingTapGetQueueTime(AudioQueueProcessingTapRef inAQTap,
		Float64* outQueueSampleTime, UInt32* outQueueFrameCount)
{
	STUB();
	return unimpErr;
}
