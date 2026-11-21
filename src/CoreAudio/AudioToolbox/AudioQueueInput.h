// Copyright (C) 2026 Lubos Dolezel
#ifndef AUDIOQUEUEINPUT_H
#define	AUDIOQUEUEINPUT_H
#include "AudioQueueBase.h"
#include <AudioToolbox/AudioComponent.h>
#include <AudioToolbox/AudioConverter.h>
#include <AudioToolbox/AudioOutputUnit.h>
#include <AudioToolbox/AudioUnitProperties.h>
#include <CoreAudio/AudioHardware.h>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

// A recording queue. It captures through the HAL output unit (AUHAL, input element), the
// same lower layer the playback queue uses for the output side, in the queue's own sample
// rate and channel count; sample formats the unit cannot deliver are converted with an
// AudioConverter. Captured frames are copied into the enqueued buffers in order. A buffer
// that has been filled is returned to the client, on the run loop or dispatch queue given
// at creation, or on the queue's own callback thread when there is none.
// Linear PCM (interleaved, or one channel) only.
class AudioQueueInput : public OpaqueAudioQueue
{
public:
	AudioQueueInput(const AudioStreamBasicDescription *inFormat, AudioQueueInputCallback inCallbackProc,
			void *inUserData, CFRunLoopRef inCallbackRunLoop, CFStringRef inCallbackRunLoopMode,
			dispatch_queue_t inCallbackQueue, AudioQueueInputCallbackBlock inCallbackBlock);
	virtual ~AudioQueueInput();

	virtual OSStatus start(const AudioTimeStamp *inStartTime) override;
	virtual OSStatus prime(UInt32 inNumberOfFramesToPrepare, UInt32 *outNumberOfFramesPrepared) override;
	virtual OSStatus flush() override;
	virtual OSStatus stop(Boolean inImmediate) override;
	virtual OSStatus pause() override;
	virtual OSStatus reset() override;
	virtual OSStatus dispose(Boolean inImmediate) override;

	virtual OSStatus getProperty(AudioQueuePropertyID inID, void *outData, UInt32 *ioDataSize) override;
	virtual OSStatus setProperty(AudioQueuePropertyID inID, const void *inData, UInt32 inDataSize) override;
	virtual OSStatus getPropertySize(AudioQueuePropertyID inID, UInt32 *outDataSize) override;

	virtual OSStatus allocateBuffer(UInt32 inBufferByteSize, UInt32 inNumberPacketDescriptions, AudioQueueBufferRef *outBuffer) override;
	virtual OSStatus freeBuffer(AudioQueueBufferRef inBuffer) override;
	virtual OSStatus enqueueBuffer(AudioQueueBufferRef inBuffer, UInt32 inNumPacketDescs, const AudioStreamPacketDescription *inPacketDescs) override;
	virtual OSStatus getCurrentTime(AudioTimeStamp *outTimeStamp, Boolean *outTimelineDiscontinuity) override;

	// A recording queue has no processing taps and cannot render offline.
	virtual OSStatus setOfflineRenderFormat(const AudioStreamBasicDescription *inFormat, const AudioChannelLayout *inLayout) override;
	virtual OSStatus offlineRender(const AudioTimeStamp *inTimestamp, AudioQueueBufferRef ioBuffer, UInt32 inNumberFrames) override;

	// Exactly one of inCallbackProc and inCallbackBlock is set; inCallbackQueue goes with the block.
	static OSStatus create(const AudioStreamBasicDescription *inFormat, AudioQueueInputCallback inCallbackProc,
			void *inUserData, CFRunLoopRef inCallbackRunLoop, CFStringRef inCallbackRunLoopMode,
			dispatch_queue_t inCallbackQueue, AudioQueueInputCallbackBlock inCallbackBlock,
			AudioQueueInput** newQueue);

private:
	struct Filled
	{
		AudioQueueBufferRef buffer;
		AudioTimeStamp start;
	};

	static OSStatus inputCallback(void* refCon, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* timeStamp,
		UInt32 busNumber, UInt32 numberFrames, AudioBufferList* ioData);
	OSStatus capture(AudioUnitRenderActionFlags* flags, const AudioTimeStamp* timeStamp, UInt32 numberFrames);
	void append(const uint8_t* data, UInt32 frames, const AudioTimeStamp* timeStamp);
	void updateLevels(const uint8_t* data, UInt32 frames);
	AudioDeviceID resolveDevice();
	Float64 deviceSampleRate(AudioDeviceID device);
	OSStatus configure();
	bool refreshRate(AudioDeviceID device);
	OSStatus ensureUnit();
	void destroyUnit();
	// Everything enqueued, in order, back to the client with what has been recorded in it.
	// Called with m_mutex held; the callbacks are issued afterwards by returnBuffers().
	void takePending(std::vector<Filled>& out);
	void returnBuffers(const std::vector<Filled>& buffers);
	void runOnClient(std::function<void()> job) override;
	void finishStop();
	OSStatus shutdown(bool deliver);
	bool onCallbackQueue() const;
	void drainCallbacks();
	bool usesCallbackThread() const { return !m_runloop && !m_dispatchQueue; }
	Float32 meterValue(Float32 linear, bool inDB) const;
private:
	AudioQueueInputCallback m_callback;
	AudioQueueInputCallbackBlock m_block = nullptr;
	dispatch_queue_t m_dispatchQueue = nullptr;

	AudioComponentInstance m_unit = nullptr;
	AudioConverterRef m_converter = nullptr;
	AudioStreamBasicDescription m_unitFormat = {};	// what the unit delivers; the queue's format unless converted
	std::vector<uint8_t> m_scratch;			// one render slice in the unit's format
	std::vector<uint8_t> m_converted;		// the same in the queue's format
	UInt32 m_maxSliceFrames = 0;
	std::atomic<UInt32> m_converterError { 0 };
	double m_hostTicksPerFrame = 0;
	bool m_rateFromDevice = false;			// created with a sample rate of 0: the device's rate is used

	// The queue's own callback thread (client callbacks when no run loop or dispatch queue was
	// given, and the work that must not run on the capture thread).
	void callbackThread();
	void post(std::function<void()> job);
	std::thread m_cbThread;
	std::mutex m_cbMutex;
	std::condition_variable m_cbCond;
	std::deque<std::function<void()>> m_cbJobs;
	bool m_cbBusy = false;
	bool m_cbQuit = false;
	bool m_selfDelete = false;

	// Callbacks posted to a run loop or dispatch queue can run after the queue is gone: each
	// holds the gate and runs only while it is open. Destruction closes it, waiting for one in
	// progress; recursive, so a dispose from inside such a callback does not deadlock.
	struct CallbackGate
	{
		std::recursive_mutex mutex;
		bool open = true;
	};
	std::shared_ptr<CallbackGate> m_gate = std::make_shared<CallbackGate>();

	// Serializes start/stop/dispose/property changes of the device. The unit's Start/Stop wait
	// for the capture thread, which takes m_mutex: they are only called under this one.
	std::mutex m_controlMutex;
	std::mutex m_mutex; // guards everything below
	std::deque<AudioQueueBufferRef> m_pending;	// enqueued; the front one is being filled
	std::set<AudioQueueBufferRef> m_allocated;
	UInt32 m_curBytes = 0;				// filled so far in m_pending.front()
	AudioTimeStamp m_curStart = {};
	bool m_running = false;
	bool m_paused = false;
	bool m_stopPending = false;			// a deferred stop is draining the buffers
	UInt64 m_frames = 0;				// frames since the queue started (paused time excluded)
	bool m_metering = false;
	std::vector<AudioQueueLevelMeterState> m_levels;	// linear, one per channel
	std::vector<float> m_levelSum;			// scratch for updateLevels

	CFStringRef m_deviceUID = nullptr;		// set by the client; NULL tracks the default input device
};

#endif	/* AUDIOQUEUEINPUT_H */
