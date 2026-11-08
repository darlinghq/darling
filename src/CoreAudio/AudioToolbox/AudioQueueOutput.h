#ifndef AUDIOQUEUEOUTPUT_H
#define	AUDIOQUEUEOUTPUT_H
#include "AudioQueueBase.h"
#include <AudioToolbox/AudioComponent.h>
#include <AudioToolbox/AudioOutputUnit.h>
#include <AudioToolbox/AudioUnitProperties.h>
#include <condition_variable>
#include <functional>
#include <thread>
#include <atomic>
#include <deque>
#include <mutex>
#include <set>
#include <vector>

// An output queue played through the default output unit (AUHAL on PulseAudio).
// Linear PCM only. Buffers are rendered in the order they were enqueued; when one
// has been fully handed to the output unit, the output callback returns it to the
// client — on the run loop given to AudioQueueNewOutput, or on the queue's own
// callback thread when that is NULL.
class AudioQueueOutput : public AudioQueue
{
public:
	AudioQueueOutput(const AudioStreamBasicDescription *inFormat,
			AudioQueueOutputCallback inCallbackProc,
		void *inUserData, CFRunLoopRef inCallbackRunLoop,
		CFStringRef inCallbackRunLoopMode, UInt32 inFlags);

	virtual ~AudioQueueOutput();

	virtual OSStatus start(const AudioTimeStamp *inStartTime) override;
	virtual OSStatus prime(UInt32 inNumberOfFramesToPrepare, UInt32 *outNumberOfFramesPrepared) override;
	virtual OSStatus flush() override;
	virtual OSStatus stop(Boolean inImmediate) override;
	virtual OSStatus pause() override;
	virtual OSStatus reset() override;

	virtual OSStatus setOfflineRenderFormat(const AudioStreamBasicDescription *inFormat, const AudioChannelLayout *inLayout) override;
	virtual OSStatus offlineRender(const AudioTimeStamp *inTimestamp, AudioQueueBufferRef ioBuffer, UInt32 inNumberFrames) override;

	virtual OSStatus dispose(Boolean inImmediate) override;

	virtual OSStatus getParameter(AudioQueueParameterID inParamID, AudioQueueParameterValue *outValue) override;
	virtual OSStatus setParameter(AudioQueueParameterID inParamID, AudioQueueParameterValue inValue) override;
	virtual OSStatus getProperty(AudioQueuePropertyID inID, void *outData, UInt32 *ioDataSize) override;
	virtual OSStatus getPropertySize(AudioQueuePropertyID inID, UInt32 *outDataSize) override;

	virtual OSStatus allocateBuffer(UInt32 inBufferByteSize, UInt32 inNumberPacketDescriptions, AudioQueueBufferRef *outBuffer) override;
	virtual OSStatus freeBuffer(AudioQueueBufferRef inBuffer) override;
	virtual OSStatus enqueueBuffer(AudioQueueBufferRef inBuffer, UInt32 inNumPacketDescs, const AudioStreamPacketDescription *inPacketDescs) override;
	virtual OSStatus getCurrentTime(AudioTimeStamp *outTimeStamp, Boolean *outTimelineDiscontinuity) override;

	static OSStatus create(const AudioStreamBasicDescription *inFormat,
			AudioQueueOutputCallback inCallbackProc,
		void *inUserData, CFRunLoopRef inCallbackRunLoop,
		CFStringRef inCallbackRunLoopMode, UInt32 inFlags,
			AudioQueueOutput** newQueue);
private:
	static OSStatus renderCallback(void* refCon, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* timeStamp,
		UInt32 busNumber, UInt32 numberFrames, AudioBufferList* ioData);
	OSStatus render(UInt32 numberFrames, AudioBufferList* ioData);
	OSStatus ensureUnit();
	void returnBuffers(const std::vector<AudioQueueBufferRef>& buffers);
	bool onCallbackQueue() const;
	void drainCallbacks();
private:
	AudioQueueOutputCallback m_callback;
	AudioComponentInstance m_unit = nullptr;

	// The queue's own callback thread, as Apple's AudioQueue has: output callbacks, deferred
	// stops and a dispose issued from a callback run there, in order. Not a dispatch queue:
	// under Darling its worker threads wake up late, so the client heard about returned
	// buffers too late to refill the queue in time (underruns).
	void callbackThread();
	void post(std::function<void()> job);
	std::thread m_cbThread;
	std::mutex m_cbMutex;
	std::condition_variable m_cbCond;
	std::deque<std::function<void()>> m_cbJobs;
	bool m_cbBusy = false;
	bool m_cbQuit = false;
	bool m_selfDelete = false; // set by a job: the thread deletes the queue after it

	// Serializes start/stop/dispose. The output unit's Start/Stop wait on the PulseAudio
	// queue, which runs render() and takes m_mutex: calling them under m_mutex deadlocks,
	// so they are only ever called under this one (render() never takes it).
	std::mutex m_controlMutex;
	std::mutex m_mutex; // guards everything below
	std::deque<AudioQueueBufferRef> m_pending;
	UInt32 m_offset = 0; // bytes of m_pending.front() already rendered
	std::set<AudioQueueBufferRef> m_allocated;
	bool m_running = false;
	bool m_paused = false;
	bool m_stopWhenDrained = false;

	std::atomic<uint64_t> m_renderedFrames { 0 };
	std::atomic<float> m_volume { 1.0f };
	double m_latencyFrames = 0;
};

#endif	/* AUDIOQUEUEOUTPUT_H */
