#ifndef AUDIOQUEUEOUTPUT_H
#define	AUDIOQUEUEOUTPUT_H
#include "AudioQueueBase.h"
#include "TimePitch.h"
#include <AudioToolbox/AudioComponent.h>
#include <CoreAudio/AudioHardware.h>
#include <AudioToolbox/AudioOutputUnit.h>
#include <AudioToolbox/AudioUnitProperties.h>
#include <condition_variable>
#include <functional>
#include <thread>
#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

// An output queue played through the default output unit (AUHAL on PulseAudio).
// Linear PCM only. Buffers are rendered in the order they were enqueued; when one
// has been fully handed to the output unit, the output callback returns it to the
// client — on the run loop given to AudioQueueNewOutput, or on the queue's own
// callback thread when that is NULL.
class AudioQueueOutput : public OpaqueAudioQueue
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

	virtual OSStatus processingTapNew(AudioQueueProcessingTapCallback inCallback, void* inClientData,
			AudioQueueProcessingTapFlags inFlags, UInt32* outMaxFrames,
			AudioStreamBasicDescription* outProcessingFormat, AudioQueueProcessingTapRef* outAQTap) override;
	virtual OSStatus processingTapDispose(AudioQueueProcessingTapRef inAQTap) override;
	virtual OSStatus processingTapGetSourceAudio(AudioQueueProcessingTapRef inAQTap, UInt32 inNumberFrames,
			AudioTimeStamp* ioTimeStamp, AudioQueueProcessingTapFlags* outFlags, UInt32* outNumberFrames,
			AudioBufferList* ioData) override;
	virtual OSStatus processingTapGetQueueTime(AudioQueueProcessingTapRef inAQTap,
			Float64* outQueueSampleTime, UInt32* outQueueFrameCount) override;

	virtual OSStatus setOfflineRenderFormat(const AudioStreamBasicDescription *inFormat, const AudioChannelLayout *inLayout) override;
	virtual OSStatus offlineRender(const AudioTimeStamp *inTimestamp, AudioQueueBufferRef ioBuffer, UInt32 inNumberFrames) override;

	virtual OSStatus dispose(Boolean inImmediate) override;

	virtual OSStatus getParameter(AudioQueueParameterID inParamID, AudioQueueParameterValue *outValue) override;
	virtual OSStatus setParameter(AudioQueueParameterID inParamID, AudioQueueParameterValue inValue) override;
	virtual OSStatus getProperty(AudioQueuePropertyID inID, void *outData, UInt32 *ioDataSize) override;
	virtual OSStatus setProperty(AudioQueuePropertyID inID, const void *inData, UInt32 inDataSize) override;
	virtual OSStatus getPropertySize(AudioQueuePropertyID inID, UInt32 *outDataSize) override;

	virtual OSStatus allocateBuffer(UInt32 inBufferByteSize, UInt32 inNumberPacketDescriptions, AudioQueueBufferRef *outBuffer) override;
	virtual OSStatus freeBuffer(AudioQueueBufferRef inBuffer) override;
	virtual OSStatus enqueueBuffer(AudioQueueBufferRef inBuffer, UInt32 inNumPacketDescs, const AudioStreamPacketDescription *inPacketDescs) override;
	virtual OSStatus enqueueBufferWithParameters(AudioQueueBufferRef inBuffer, UInt32 inNumPacketDescs,
			const AudioStreamPacketDescription *inPacketDescs, UInt32 inTrimFramesAtStart, UInt32 inTrimFramesAtEnd,
			UInt32 inNumParamValues, const AudioQueueParameterEvent *inParamValues,
			const AudioTimeStamp *inStartTime, AudioTimeStamp *outActualStartTime) override;
	virtual OSStatus getCurrentTime(AudioTimeStamp *outTimeStamp, Boolean *outTimelineDiscontinuity) override;

	static OSStatus create(const AudioStreamBasicDescription *inFormat,
			AudioQueueOutputCallback inCallbackProc,
		void *inUserData, CFRunLoopRef inCallbackRunLoop,
		CFStringRef inCallbackRunLoopMode, UInt32 inFlags,
			AudioQueueOutput** newQueue);
private:
	static OSStatus renderCallback(void* refCon, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* timeStamp,
		UInt32 busNumber, UInt32 numberFrames, AudioBufferList* ioData);
	// `produced` (optional) receives the number of frames of the first buffer that carry audio.
	OSStatus render(UInt32 numberFrames, AudioBufferList* ioData, UInt32* produced = nullptr);
	// The time/pitch effect is on and changes the audio, or still holds some. Called with m_mutex held.
	bool timePitchInUseLocked() const;
	// Renders a block through the time/pitch effect; returns the frames that carry audio.
	UInt32 renderTimePitchLocked(uint8_t* out, UInt32 want, std::vector<AudioQueueBufferRef>& finished);
	void applyTimePitchLocked();
	void destroyTap();
	OSStatus renderTapped(UInt32 numberFrames, AudioBufferList* ioData);
	void finishRender(std::vector<AudioQueueBufferRef>& finished, UInt32 numberFrames, double speed = 1.0);
	struct Tap;
	OSStatus pullTapSource(Tap* tap, UInt32 inNumberFrames, AudioQueueProcessingTapFlags* outFlags,
			UInt32* outNumberFrames, AudioBufferList* ioData);
	struct Pending
	{
		AudioQueueBufferRef buffer;
		UInt32 startByte;	// what AudioQueueEnqueueBufferWithParameters trimmed at the start
		UInt32 endByte;		// end of the audio to play (the end trim removed)
		bool hasStart = false;
		Float64 startSample = 0; // on the queue's timeline
		std::vector<AudioQueueParameterEvent> events;
		bool begun = false;	// start time and parameter events have been acted on
	};
	// Pulls up to wantBytes of what has been enqueued; queuePos is the queue time (in frames) of the
	// first frame written. Called with m_mutex held.
	UInt32 pullLocked(uint8_t* out, UInt32 wantBytes, std::vector<AudioQueueBufferRef>& finished, Float64 queuePos);
	void beginEntryLocked(Pending& entry, Float64 queuePos);
	UInt64 pendingFramesLocked() const;
	void applyEntryGain(uint8_t* data, UInt32 frames);
	void applyQueueGain(uint8_t* data, UInt32 frames);
	void updateLevels(const uint8_t* data, UInt32 frames);
	void runOnClient(std::function<void()> job) override;
	void notifyRunning();
	AudioDeviceID resolveDevice();
	// Frames, at the queue's sample rate, that the output device holds between rendering and playing.
	double deviceLatencyFrames();
	double playoutSeconds();
	void scheduleDrainStop();
	OSStatus deviceUID(CFStringRef* outUID);
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
	std::atomic<bool> m_disposing { false };
	bool m_selfDelete = false; // set by a job: the thread deletes the queue after it

	// Callbacks posted to a client run loop can run after the queue is gone: each one holds
	// the gate and runs only while it is open. Destruction closes it, waiting for a callback
	// in progress; recursive, so a dispose from inside a run-loop callback does not deadlock.
	struct CallbackGate
	{
		std::recursive_mutex mutex;
		bool open = true;
	};
	std::shared_ptr<CallbackGate> m_gate = std::make_shared<CallbackGate>();

	// Serializes start/stop/dispose. The output unit's Start/Stop wait on the PulseAudio
	// queue, which runs render() and takes m_mutex: calling them under m_mutex deadlocks,
	// so they are only ever called under this one (render() never takes it).
	std::mutex m_controlMutex;
	std::mutex m_mutex; // guards everything below
	std::deque<Pending> m_pending;
	UInt32 m_offset = 0; // byte position in m_pending.front()'s buffer, once it has begun
	UInt64 m_gapFrames = 0; // silence still to play before the front entry's audio (a future start time)

	// Volume and pan given with the enqueued buffers: a volume change ramps linearly to its target.
	float m_entryGain = 1.0f, m_entryTarget = 1.0f, m_entryStep = 0.0f;
	UInt64 m_entryRampLeft = 0;
	float m_entryPan = 0.0f;
	std::atomic<float> m_pan { 0.0f }; // the queue's own pan parameter
	// The play rate and pitch (cents) are acted on by the time/pitch effect, when it is enabled;
	// the volume ramp time is only stored.
	float m_playRate = 1.0f, m_pitch = 0.0f, m_volumeRampTime = 0.0f;

	bool m_metering = false;
	std::vector<AudioQueueLevelMeterState> m_levels;
	CFStringRef m_deviceUID = nullptr;
	UInt32 m_decodeBufferFrames = 0;
	UInt32 m_timePitchEnabled = 0, m_timePitchAlgorithm = kAudioQueueTimePitchAlgorithm_Spectral, m_timePitchBypass = 0;
	// The time/pitch effect; created when it first has something to do.
	std::unique_ptr<TimePitchProcessor> m_timePitch;
	bool m_flushRequested = false;	// AudioQueueFlush was called and what was enqueued has not played out
	bool m_primed = false;		// AudioQueuePrime was called, and AudioQueueReset has not been since
	bool m_everStarted = false;
	// Media time (in frames) that the frames rendered so far stand for: equals m_renderedFrames unless
	// the effect changed the speed. The queue's timeline, and so AudioQueueGetCurrentTime, is in it.
	double m_mediaRendered = 0.0;
	double m_blockSpeed = 1.0;	// media frames per frame rendered in the block being rendered
	std::vector<std::vector<float>> m_tpOut;
	std::vector<uint8_t> m_tpBytes;
	std::vector<uint8_t> m_channelLayout;
	std::set<AudioQueueBufferRef> m_allocated;
	bool m_running = false;
	bool m_paused = false;
	bool m_stopWhenDrained = false;

	bool m_streamStart = true; // the next audio pulled starts a continuous stream (tap flag)

	// Offline rendering (AudioQueueSetOfflineRenderFormat): the queue runs without the
	// output unit and AudioQueueOfflineRender pulls its audio in the offline format.
	std::atomic<bool> m_offline { false };
	AudioStreamBasicDescription m_offlineFormat = {};

	// The processing tap: one per queue, created and disposed only while the queue is
	// stopped. m_tapMutex is held for as long as the tap's callback runs, so disposing
	// the tap waits for a callback in progress. Taken before m_mutex, never after.
	std::mutex m_tapMutex;
	Tap* m_tap = nullptr;

	std::atomic<uint64_t> m_renderedFrames { 0 };
	std::atomic<float> m_volume { 1.0f };
	double m_latencyFrames = 0;
	// The size and time of the last block the output unit took (guarded by m_mutex), to tell how much of
	// what was rendered is still on its way to the device.
	UInt32 m_lastBlockFrames = 0;
	uint64_t m_lastBlockNanos = 0;
};

#endif	/* AUDIOQUEUEOUTPUT_H */
