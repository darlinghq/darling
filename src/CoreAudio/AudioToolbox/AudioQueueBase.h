#ifndef AUDIOQUEUEBASE_H
#define	AUDIOQUEUEBASE_H
#include "AudioQueue.h"
#include <functional>
#include <mutex>
#include <vector>

struct OpaqueAudioQueue;

// A processing tap. The queue that created it owns it; the public tap functions
// are forwarded to that queue.
struct OpaqueAudioQueueProcessingTap
{
	explicit OpaqueAudioQueueProcessingTap(OpaqueAudioQueue* q) : queue(q) {}
	virtual ~OpaqueAudioQueueProcessingTap() {}
	OpaqueAudioQueue* const queue;
};

struct OpaqueAudioQueue
{
public:
	OpaqueAudioQueue(const AudioStreamBasicDescription* format, void* userData,
			CFRunLoopRef runloop, CFStringRef runloopMode, UInt32 flags);
	virtual ~OpaqueAudioQueue();
	
	virtual OSStatus start(const AudioTimeStamp *inStartTime) = 0;
	virtual OSStatus prime(UInt32 inNumberOfFramesToPrepare, UInt32 *outNumberOfFramesPrepared) = 0;
	virtual OSStatus flush() = 0;
	virtual OSStatus stop(Boolean inImmediate) = 0;
	virtual OSStatus pause() = 0;
	virtual OSStatus reset() = 0;

	virtual OSStatus dispose(Boolean inImmediate) = 0;

	virtual OSStatus getParameter(AudioQueueParameterID inParamID, AudioQueueParameterValue *outValue);
	virtual OSStatus setParameter(AudioQueueParameterID inParamID, AudioQueueParameterValue inValue);

	virtual OSStatus getProperty(AudioQueuePropertyID inID, void *outData, UInt32 *ioDataSize);
	virtual OSStatus setProperty(AudioQueuePropertyID inID, const void *inData, UInt32 inDataSize);
	virtual OSStatus getPropertySize(AudioQueuePropertyID inID, UInt32 *outDataSize);

	virtual OSStatus addPropertyListener(AudioQueuePropertyID inID, AudioQueuePropertyListenerProc inProc, void *inUserData);
	virtual OSStatus removePropertyListener(AudioQueuePropertyID inID, AudioQueuePropertyListenerProc inProc, void *inUserData);

	// Not every queue type has more to say than the plain enqueue; the ones that do override this.
	virtual OSStatus enqueueBufferWithParameters(AudioQueueBufferRef inBuffer, UInt32 inNumPacketDescs,
			const AudioStreamPacketDescription *inPacketDescs, UInt32 inTrimFramesAtStart, UInt32 inTrimFramesAtEnd,
			UInt32 inNumParamValues, const AudioQueueParameterEvent *inParamValues,
			const AudioTimeStamp *inStartTime, AudioTimeStamp *outActualStartTime);

	virtual OSStatus allocateBuffer(UInt32 inBufferByteSize, UInt32 inNumberPacketDescriptions, AudioQueueBufferRef *outBuffer);
	virtual OSStatus freeBuffer(AudioQueueBufferRef inBuffer);
	virtual OSStatus enqueueBuffer(AudioQueueBufferRef inBuffer, UInt32 inNumPacketDescs, const AudioStreamPacketDescription *inPacketDescs);
	virtual OSStatus getCurrentTime(AudioTimeStamp *outTimeStamp, Boolean *outTimelineDiscontinuity);

	virtual OSStatus processingTapNew(AudioQueueProcessingTapCallback inCallback, void* inClientData,
			AudioQueueProcessingTapFlags inFlags, UInt32* outMaxFrames,
			AudioStreamBasicDescription* outProcessingFormat, AudioQueueProcessingTapRef* outAQTap);
	virtual OSStatus processingTapDispose(AudioQueueProcessingTapRef inAQTap);
	virtual OSStatus processingTapGetSourceAudio(AudioQueueProcessingTapRef inAQTap, UInt32 inNumberFrames,
			AudioTimeStamp* ioTimeStamp, AudioQueueProcessingTapFlags* outFlags, UInt32* outNumberFrames,
			AudioBufferList* ioData);
	virtual OSStatus processingTapGetQueueTime(AudioQueueProcessingTapRef inAQTap,
			Float64* outQueueSampleTime, UInt32* outQueueFrameCount);

	virtual OSStatus setOfflineRenderFormat(const AudioStreamBasicDescription *inFormat, const AudioChannelLayout *inLayout) = 0;
	virtual OSStatus offlineRender(const AudioTimeStamp *inTimestamp, AudioQueueBufferRef ioBuffer, UInt32 inNumberFrames) = 0;
protected:
	// Runs the job where the queue's client callbacks run: on the run loop given at
	// creation, or else on the queue's own callback thread (or dispatch queue).
	virtual void runOnClient(std::function<void()> job) = 0;
	// Tells the listeners registered for a property that it changed, through runOnClient().
	void notifyListeners(AudioQueuePropertyID id);

	AudioStreamBasicDescription m_format;
	void* m_userData;
	CFRunLoopRef m_runloop;
	CFStringRef m_runloopMode;
	UInt32 m_flags;
private:
	struct Listener
	{
		AudioQueuePropertyID id;
		AudioQueuePropertyListenerProc proc;
		void* userData;
	};
	std::mutex m_listenerMutex;
	std::vector<Listener> m_listeners;
};

#endif	/* AUDIOQUEUEBASE_H */

