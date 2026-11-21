#include "AudioQueueOutput.h"
#include "stub.h"
#include <CarbonCore/MacErrors.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioFormat.h>
#include <CoreAudio/AudioHardware.h>
#include <mach/mach_time.h>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ctime>

static const UInt32 kDeviceBufferFrames = 2048;

// Status codes AudioQueue returns for the time/pitch properties that AudioQueue.h does not name; see
// the QUIRK comment at AudioQueueOutput::setProperty.
static const OSStatus kTimePitchNotAllowed = -66629;
static const OSStatus kTimePitchInvalidValue = -66630;
// The deprecated low-quality, zero-latency algorithm: recognized by the queue, but refused.
static const UInt32 kAudioQueueTimePitchAlgorithm_LowQualityZeroLatency = 'lqzl';

static uint64_t monotonicNanos()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return uint64_t(ts.tv_sec) * 1000000000ull + uint64_t(ts.tv_nsec);
}

AudioQueueOutput::AudioQueueOutput(const AudioStreamBasicDescription *inFormat,
		AudioQueueOutputCallback inCallbackProc,
	void *inUserData, CFRunLoopRef inCallbackRunLoop,
	CFStringRef inCallbackRunLoopMode, UInt32 inFlags)
: OpaqueAudioQueue(inFormat, inUserData, inCallbackRunLoop, inCallbackRunLoopMode, inFlags),
		m_callback(inCallbackProc)
{
	m_cbThread = std::thread(&AudioQueueOutput::callbackThread, this);
}

void AudioQueueOutput::callbackThread()
{
	for (;;)
	{
		std::function<void()> job;
		{
			std::unique_lock<std::mutex> lock(m_cbMutex);
			m_cbCond.wait(lock, [&] { return !m_cbJobs.empty() || m_cbQuit; });
			if (m_cbJobs.empty())
				return;
			job = std::move(m_cbJobs.front());
			m_cbJobs.pop_front();
			m_cbBusy = true;
		}
		job();
		if (m_selfDelete)
		{
			// A dispose issued from a callback: the queue goes now, after that callback.
			delete this;
			return;
		}
		{
			std::lock_guard<std::mutex> lock(m_cbMutex);
			m_cbBusy = false;
		}
		m_cbCond.notify_all();
	}
}

void AudioQueueOutput::post(std::function<void()> job)
{
	{
		std::lock_guard<std::mutex> lock(m_cbMutex);
		m_cbJobs.push_back(std::move(job));
	}
	m_cbCond.notify_all();
}

AudioQueueOutput::~AudioQueueOutput()
{
	{
		std::lock_guard<std::recursive_mutex> lock(m_gate->mutex);
		m_gate->open = false;
	}
	if (m_unit)
	{
		AudioOutputUnitStop(m_unit);
		AudioUnitUninitialize(m_unit);
		AudioComponentInstanceDispose(m_unit);
	}
	drainCallbacks();
	{
		std::lock_guard<std::mutex> lock(m_cbMutex);
		m_cbQuit = true;
	}
	m_cbCond.notify_all();
	if (onCallbackQueue())
		m_cbThread.detach(); // deleted by its own thread (dispose from a callback)
	else
		m_cbThread.join();
	destroyTap(); // a tap is destroyed with its queue
	if (m_deviceUID)
		CFRelease(m_deviceUID);
	for (AudioQueueBufferRef buffer : m_allocated)
	{
		free(buffer->mAudioData);
		free(buffer->mPacketDescriptions);
		free(buffer);
	}
}

OSStatus AudioQueueOutput::create(const AudioStreamBasicDescription *inFormat,
		AudioQueueOutputCallback inCallbackProc,
		void *inUserData, CFRunLoopRef inCallbackRunLoop,
		CFStringRef inCallbackRunLoopMode, UInt32 inFlags,
			AudioQueueOutput** newQueue)
{
	if (!inFormat || !inCallbackProc || !newQueue)
		return kAudioQueueErr_InvalidParameter;
	// Only interleaved (or mono) linear PCM: compressed formats would need a
	// converter in front of the output unit.
	if (inFormat->mFormatID != kAudioFormatLinearPCM || inFormat->mBytesPerFrame == 0
			|| inFormat->mChannelsPerFrame == 0 || inFormat->mSampleRate <= 0)
		return kAudioFormatUnsupportedDataFormatError;
	if ((inFormat->mFormatFlags & kAudioFormatFlagIsNonInterleaved) && inFormat->mChannelsPerFrame > 1)
		return kAudioFormatUnsupportedDataFormatError;

	*newQueue = new AudioQueueOutput(inFormat, inCallbackProc, inUserData,
			inCallbackRunLoop, inCallbackRunLoopMode,
			inFlags);
	return noErr;
}

OSStatus AudioQueueOutput::ensureUnit()
{
	if (m_unit)
		return noErr;

	// A device chosen with kAudioQueueProperty_CurrentDevice is played through the HAL output unit
	// set to it; otherwise the default output unit follows the user's default device.
	AudioDeviceID device = kAudioObjectUnknown;
	bool chosen;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		chosen = m_deviceUID != nullptr;
	}
	if (chosen)
	{
		device = resolveDevice();
		if (device == kAudioObjectUnknown)
			return kAudioQueueErr_InvalidDevice;
	}

	AudioComponentDescription desc = { kAudioUnitType_Output, chosen ? kAudioUnitSubType_HALOutput : kAudioUnitSubType_DefaultOutput,
		kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponent component = AudioComponentFindNext(nullptr, &desc);
	if (!component)
		return kAudioQueueErr_InvalidDevice;

	AudioComponentInstance unit;
	OSStatus status = AudioComponentInstanceNew(component, &unit);
	if (status != noErr)
		return status;
	if (chosen)
	{
		status = AudioUnitSetProperty(unit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &device, sizeof(device));
		if (status != noErr)
		{
			AudioComponentInstanceDispose(unit);
			return kAudioQueueErr_InvalidDevice;
		}
	}

	// The device's IO block. PulseAudio is given three of them, and with the default
	// (1024 frames, ~64 ms in all) it runs dry under load. A queue promises no low
	// latency, so it asks for at least kDeviceBufferFrames, never less than what is set.
	{
		UInt32 frames = 0;
		UInt32 size = sizeof(frames);
		if (AudioUnitGetProperty(unit, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 0, &frames, &size) != noErr
				|| frames < kDeviceBufferFrames)
		{
			frames = kDeviceBufferFrames;
			AudioUnitSetProperty(unit, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 0, &frames, sizeof(frames));
		}
	}

	AURenderCallbackStruct callback = { renderCallback, this };
	status = AudioUnitSetProperty(unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &m_format, sizeof(m_format));
	if (status == noErr)
		status = AudioUnitSetProperty(unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callback, sizeof(callback));
	if (status == noErr)
		status = AudioUnitInitialize(unit);
	if (status != noErr)
	{
		AudioComponentInstanceDispose(unit);
		return status;
	}

	// What the output unit holds after it has taken a buffer from us: the queue's
	// position is what has been heard, not what has been handed over.
	Float64 latency = 0;
	UInt32 size = sizeof(latency);
	if (AudioUnitGetProperty(unit, kAudioUnitProperty_Latency, kAudioUnitScope_Global, 0, &latency, &size) == noErr && latency > 0)
		m_latencyFrames = latency * m_format.mSampleRate;

	m_unit = unit;
	return noErr;
}

OSStatus AudioQueueOutput::renderCallback(void* refCon, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* timeStamp,
	UInt32 busNumber, UInt32 numberFrames, AudioBufferList* ioData)
{
	return static_cast<AudioQueueOutput*>(refCon)->render(numberFrames, ioData);
}

UInt32 AudioQueueOutput::pullLocked(uint8_t* out, UInt32 want, std::vector<AudioQueueBufferRef>& finished, Float64 queuePos)
{
	const UInt32 frameBytes = m_format.mBytesPerFrame;
	UInt32 done = 0;
	while (!m_paused && done < want && !m_pending.empty())
	{
		Pending& entry = m_pending.front();
		if (!entry.begun)
			beginEntryLocked(entry, queuePos + done / frameBytes);
		if (m_gapFrames)
		{
			// A start time ahead of the queue's position: silence until then.
			UInt32 frames = UInt32(std::min<UInt64>(m_gapFrames, (want - done) / frameBytes));
			if (!frames)
				break;
			memset(out + done, 0, size_t(frames) * frameBytes);
			done += frames * frameBytes;
			m_gapFrames -= frames;
			continue;
		}
		UInt32 left = entry.endByte - m_offset;
		UInt32 n = std::min(left, want - done);
		memcpy(out + done, static_cast<uint8_t*>(entry.buffer->mAudioData) + m_offset, n);
		applyEntryGain(out + done, n / frameBytes);
		done += n;
		m_offset += n;
		if (m_offset >= entry.endByte)
		{
			finished.push_back(entry.buffer);
			m_pending.pop_front();
			m_offset = 0;
		}
	}
	return done;
}

OSStatus AudioQueueOutput::render(UInt32 numberFrames, AudioBufferList* ioData, UInt32* produced)
{
	// A queue with a processing tap plays its audio as it is: the time/pitch effect is not applied.
	{
		std::lock_guard<std::mutex> tapLock(m_tapMutex);
		if (m_tap)
			return renderTapped(numberFrames, ioData);
	}

	std::vector<AudioQueueBufferRef> finished;
	const UInt32 frameBytes = m_format.mBytesPerFrame;
	double effectSpeed = 1.0;
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		const bool effect = ioData->mNumberBuffers == 1 && timePitchInUseLocked();
		if (effect)
			applyTimePitchLocked();
		for (UInt32 b = 0; b < ioData->mNumberBuffers; b++)
		{
			uint8_t* out = static_cast<uint8_t*>(ioData->mBuffers[b].mData);
			UInt32 want = std::min<UInt32>(ioData->mBuffers[b].mDataByteSize, numberFrames * frameBytes);
			UInt32 done = effect ? renderTimePitchLocked(out, want, finished) : pullLocked(out, want, finished, m_mediaRendered);
			if (b == 0 && produced)
				*produced = done / frameBytes;
			// Underrun or pause: silence (zero is silence for float and signed PCM).
			if (done < want)
				memset(out + done, 0, want - done);
			ioData->mBuffers[b].mDataByteSize = want;

			applyQueueGain(out, done / frameBytes);
			updateLevels(out, want / frameBytes);
		}
		if (effect)
			effectSpeed = m_blockSpeed;
	}
	finishRender(finished, numberFrames, effectSpeed);
	return noErr;
}

void AudioQueueOutput::finishRender(std::vector<AudioQueueBufferRef>& finished, UInt32 numberFrames, double speed)
{
	bool stopNow;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_paused)
		{
			m_renderedFrames.fetch_add(numberFrames, std::memory_order_relaxed);
			m_mediaRendered += double(numberFrames) * speed;
			m_blockSpeed = speed;
			m_lastBlockFrames = numberFrames;
			m_lastBlockNanos = monotonicNanos();
		}
		const bool drained = m_pending.empty() && (!m_timePitch || m_timePitch->idle());
		stopNow = m_stopWhenDrained && drained;
		if (drained)
			m_flushRequested = false; // played out: the next audio is a new stream
	}

	returnBuffers(finished);

	if (stopNow)
		scheduleDrainStop();
}

// How long what has been handed to the output unit takes to leave the device.
double AudioQueueOutput::playoutSeconds()
{
	const double deviceLatency = deviceLatencyFrames();
	std::lock_guard<std::mutex> lock(m_mutex);
	double seconds = (std::max(deviceLatency, m_latencyFrames) + m_lastBlockFrames) / m_format.mSampleRate;
	if (!m_paused && m_lastBlockNanos)
		seconds -= double(monotonicNanos() - m_lastBlockNanos) / 1e9;
	return std::min(std::max(seconds, 0.0), 1.0);
}

void AudioQueueOutput::scheduleDrainStop()
{
	// A deferred AudioQueueStop: everything enqueued has been handed to the unit. Stopping the unit
	// from its own render thread is not allowed, so it happens on the callback queue, and only after
	// the device has played what it holds: the stop would cut it off.
	post([this] {
		const double wait = playoutSeconds();
		if (wait > 0)
			std::this_thread::sleep_for(std::chrono::duration<double>(wait));

		std::lock_guard<std::mutex> control(m_controlMutex);
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_stopWhenDrained || !m_running)
				return;
			m_running = false;
			m_stopWhenDrained = false;
		}
		if (m_unit)
			AudioOutputUnitStop(m_unit);
		notifyRunning();
	});
}

// ---- Linear PCM sample access, for the processing tap and offline rendering ----

namespace
{
	struct SampleLayout
	{
		bool isFloat;
		UInt32 bytes;		// per sample
		UInt32 channels;
		UInt32 frameBytes;
		UInt32 chunkStep;	// bytes between channels of one frame (0 when the data is one channel)
	};

	// Native-endian packed integer (16, 24, 32 bit) or float (32, 64 bit) PCM, interleaved,
	// or a single non-interleaved channel.
	bool describeSamples(const AudioStreamBasicDescription& f, SampleLayout& l)
	{
		if (f.mFormatID != kAudioFormatLinearPCM || f.mChannelsPerFrame == 0 || f.mBitsPerChannel == 0
				|| f.mBitsPerChannel % 8 || (f.mFormatFlags & kAudioFormatFlagIsBigEndian)
				|| f.mFramesPerPacket != 1)
			return false;
		l.isFloat = f.mFormatFlags & kAudioFormatFlagIsFloat;
		l.bytes = f.mBitsPerChannel / 8;
		l.channels = f.mChannelsPerFrame;
		if (l.isFloat ? (l.bytes != 4 && l.bytes != 8)
				: (!(f.mFormatFlags & kAudioFormatFlagIsSignedInteger) || (l.bytes != 2 && l.bytes != 3 && l.bytes != 4)))
			return false;
		bool nonInterleaved = f.mFormatFlags & kAudioFormatFlagIsNonInterleaved;
		if (nonInterleaved && l.channels > 1)
			return false;
		l.frameBytes = nonInterleaved ? l.bytes : l.bytes * l.channels;
		l.chunkStep = l.bytes;
		return f.mBytesPerFrame == l.frameBytes;
	}

	float loadSample(const uint8_t* p, const SampleLayout& l)
	{
		if (l.isFloat)
		{
			if (l.bytes == 4)
			{
				float v;
				memcpy(&v, p, 4);
				return v;
			}
			double v;
			memcpy(&v, p, 8);
			return float(v);
		}
		int32_t v;
		switch (l.bytes)
		{
			case 2:
			{
				int16_t s;
				memcpy(&s, p, 2);
				return float(s) / 32768.0f;
			}
			case 3:
				v = int32_t(uint32_t(p[0]) << 8 | uint32_t(p[1]) << 16 | uint32_t(p[2]) << 24) >> 8;
				return float(v) / 8388608.0f;
			default:
				memcpy(&v, p, 4);
				return float(double(v) / 2147483648.0);
		}
	}

	void storeSample(uint8_t* p, const SampleLayout& l, float x)
	{
		if (l.isFloat)
		{
			if (l.bytes == 4)
				memcpy(p, &x, 4);
			else
			{
				double d = x;
				memcpy(p, &d, 8);
			}
			return;
		}
		if (!(x <= 1.0f))
			x = 1.0f; // also catches NaN
		if (x < -1.0f)
			x = -1.0f;
		switch (l.bytes)
		{
			case 2:
			{
				int16_t s = int16_t(std::lrintf(std::min(x * 32768.0f, 32767.0f)));
				memcpy(p, &s, 2);
				break;
			}
			case 3:
			{
				int32_t v = int32_t(std::lrintf(std::min(x * 8388608.0f, 8388607.0f)));
				p[0] = uint8_t(v);
				p[1] = uint8_t(v >> 8);
				p[2] = uint8_t(v >> 16);
				break;
			}
			default:
			{
				int32_t v = int32_t(std::lrint(std::min(double(x) * 2147483648.0, 2147483647.0)));
				memcpy(p, &v, 4);
			}
		}
	}

	void unpackSamples(const uint8_t* raw, const SampleLayout& l, UInt32 frames, float* const* planes, float gain)
	{
		for (UInt32 c = 0; c < l.channels; c++)
			for (UInt32 i = 0; i < frames; i++)
				planes[c][i] = loadSample(raw + i * l.frameBytes + c * l.chunkStep, l) * gain;
	}

	// planes[c] may be null (silence).
	void packSamples(uint8_t* raw, const SampleLayout& l, UInt32 frames, const float* const* planes, float gain)
	{
		for (UInt32 c = 0; c < l.channels; c++)
			for (UInt32 i = 0; i < frames; i++)
				storeSample(raw + i * l.frameBytes + c * l.chunkStep, l, planes[c] ? planes[c][i] * gain : 0.0f);
	}

	const UInt32 kTapMaxFrames = 2048;
}

// ---- the time/pitch effect ----

bool AudioQueueOutput::timePitchInUseLocked() const
{
	SampleLayout layout;
	if (!describeSamples(m_format, layout))
		return false;
	// Audio the effect still holds is rendered through it even when it was switched off meanwhile.
	if (m_timePitch && !m_timePitch->idle())
		return true;
	if (!m_timePitchEnabled || m_timePitchBypass)
		return false;
	return !TimePitchProcessor::isNeutral(m_timePitchAlgorithm == kAudioQueueTimePitchAlgorithm_Varispeed ? TimePitchProcessor::Varispeed
			: m_timePitchAlgorithm == kAudioQueueTimePitchAlgorithm_TimeDomain ? TimePitchProcessor::TimeDomain : TimePitchProcessor::Spectral,
			m_playRate, m_pitch);
}

void AudioQueueOutput::applyTimePitchLocked()
{
	SampleLayout layout;
	describeSamples(m_format, layout);
	if (!m_timePitch)
		m_timePitch.reset(new TimePitchProcessor(layout.channels, m_format.mSampleRate));
	const bool wanted = m_timePitchEnabled && !m_timePitchBypass;
	const TimePitchProcessor::Algorithm algorithm = m_timePitchAlgorithm == kAudioQueueTimePitchAlgorithm_Varispeed ? TimePitchProcessor::Varispeed
			: m_timePitchAlgorithm == kAudioQueueTimePitchAlgorithm_TimeDomain ? TimePitchProcessor::TimeDomain : TimePitchProcessor::Spectral;
	m_timePitch->configure(algorithm, wanted ? m_playRate : 1.0, wanted ? m_pitch : 0.0);
}

UInt32 AudioQueueOutput::renderTimePitchLocked(uint8_t* out, UInt32 want, std::vector<AudioQueueBufferRef>& finished)
{
	SampleLayout layout;
	describeSamples(m_format, layout);
	const UInt32 frames = want / layout.frameBytes;

	// What the effect reads: the queue's audio as the buffers' owner enqueued it (with the volume
	// ramps and pan of the enqueued buffers), as planar float. The queue's own volume and pan come
	// after the effect.
	struct Feed : TimePitchProcessor::Source
	{
		AudioQueueOutput* queue;
		const SampleLayout& layout;
		std::vector<AudioQueueBufferRef>& finished;
		Float64 position;	// the queue's time of the next frame pulled
		Feed(AudioQueueOutput* q, const SampleLayout& l, std::vector<AudioQueueBufferRef>& f, Float64 p)
		: queue(q), layout(l), finished(f), position(p) {}
		size_t pull(float* const* planes, size_t count) override
		{
			queue->m_tpBytes.resize(count * layout.frameBytes);
			const UInt32 got = queue->pullLocked(queue->m_tpBytes.data(), UInt32(queue->m_tpBytes.size()), finished, position) / layout.frameBytes;
			position += got;
			unpackSamples(queue->m_tpBytes.data(), layout, got, planes, 1.0f);
			return got;
		}
		// The enqueued audio is the whole stream when the queue renders offline, was told to stop once it has played
		// what it holds, or was flushed; otherwise the client is late with the next buffer.
		bool endOfStream() override { return queue->m_offline || queue->m_stopWhenDrained || queue->m_flushRequested; }
	} feed(this, layout, finished, m_mediaRendered);

	m_tpOut.resize(layout.channels);
	std::vector<float*> planes(layout.channels);
	std::vector<const float*> cplanes(layout.channels);
	for (UInt32 c = 0; c < layout.channels; c++)
	{
		m_tpOut[c].resize(frames);
		planes[c] = m_tpOut[c].data();
		cplanes[c] = planes[c];
	}
	const UInt32 got = UInt32(m_timePitch->render(planes.data(), frames, feed));
	packSamples(out, layout, got, cplanes.data(), 1.0f);
	m_blockSpeed = m_timePitch->speed();
	return got * layout.frameBytes;
}

struct AudioQueueOutput::Tap : OpaqueAudioQueueProcessingTap
{
	Tap(OpaqueAudioQueue* q, const SampleLayout& l, AudioQueueProcessingTapCallback cb, void* data,
			AudioQueueProcessingTapFlags f)
	: OpaqueAudioQueueProcessingTap(q), layout(l), callback(cb), clientData(data), flags(f),
		raw(size_t(kTapMaxFrames) * l.frameBytes), source(size_t(kTapMaxFrames) * l.channels),
		listMemory(offsetof(AudioBufferList, mBuffers) + l.channels * sizeof(AudioBuffer))
	{
	}

	AudioBufferList* list() { return reinterpret_cast<AudioBufferList*>(listMemory.data()); }
	float* plane(UInt32 c) { return source.data() + size_t(c) * kTapMaxFrames; }

	SampleLayout layout;
	AudioQueueProcessingTapCallback callback;
	void* clientData;
	AudioQueueProcessingTapFlags flags;
	std::vector<uint8_t> raw;		// source audio in the queue's format
	std::vector<float> source;		// the same, in the processing format
	std::vector<uint8_t> listMemory;

	// Only while the callback runs (under m_tapMutex)
	bool active = false;
	std::thread::id thread;
	Float64 chunkTime = 0;			// queue sample time of the chunk
	UInt32 chunkFrames = 0;
	bool paused = false;
	bool startOfStream = false;		// this chunk begins a continuous stream
	bool sourced = false;			// the callback has pulled source audio
	std::vector<AudioQueueBufferRef> finished;	// buffers fully consumed by this render
};

void AudioQueueOutput::destroyTap()
{
	delete m_tap;
	m_tap = nullptr;
}

OSStatus AudioQueueOutput::renderTapped(UInt32 numberFrames, AudioBufferList* ioData)
{
	// Where the tap sits: the queue's audio is converted to the processing format (float,
	// one buffer per channel), and the tap is called on chunks of at most kTapMaxFrames. The result is
	// converted back to the queue's format. A tap that returns fewer frames than the
	// chunk leaves the rest of the chunk silent. The queue has no effects of its own, so
	// pre- and post-effects taps sit at the same point; the queue volume is applied to
	// what comes out of the tap, so a tap always sees the unattenuated audio.
	Tap* tap = m_tap;
	const SampleLayout& l = tap->layout;
	const float volume = m_volume.load(std::memory_order_relaxed);
	const bool siphon = tap->flags & kAudioQueueProcessingTap_Siphon;

	uint8_t* out = static_cast<uint8_t*>(ioData->mBuffers[0].mData);
	const UInt32 wantFrames = std::min<UInt32>(ioData->mBuffers[0].mDataByteSize, numberFrames * l.frameBytes) / l.frameBytes;
	const uint64_t baseTime = m_renderedFrames.load(std::memory_order_relaxed);

	tap->finished.clear();
	tap->active = true;
	tap->thread = std::this_thread::get_id();

	UInt32 done = 0;
	while (done < wantFrames)
	{
		const UInt32 chunk = std::min(wantFrames - done, kTapMaxFrames);
		uint8_t* chunkOut = out + size_t(done) * l.frameBytes;
		tap->chunkTime = Float64(baseTime + done);
		tap->chunkFrames = chunk;
		{
			// Playback beginning or resuming is reported with the first chunk after it, to
			// every source pull of that chunk.
			std::lock_guard<std::mutex> lock(m_mutex);
			tap->paused = m_paused;
			tap->startOfStream = false;
			if (!m_paused && m_streamStart)
			{
				tap->startOfStream = true;
				m_streamStart = false;
			}
		}
		tap->sourced = false;

		AudioTimeStamp stamp;
		memset(&stamp, 0, sizeof(stamp));
		stamp.mSampleTime = tap->chunkTime;
		stamp.mFlags = kAudioTimeStampSampleTimeValid;
		AudioQueueProcessingTapFlags flags = tap->flags;
		UInt32 produced = 0;
		AudioBufferList* list = tap->list();
		list->mNumberBuffers = l.channels;

		if (siphon)
		{
			// The tap observes the source audio; what the queue plays is that audio itself.
			UInt32 got = 0;
			AudioQueueProcessingTapFlags pullFlags = 0;
			for (UInt32 c = 0; c < l.channels; c++)
				list->mBuffers[c] = { 1, chunk * UInt32(sizeof(float)), tap->plane(c) };
			pullTapSource(tap, chunk, &pullFlags, &got, list);
			tap->sourced = false; // a siphon's callback has not asked for source audio
			flags |= pullFlags;
			UInt32 zero = 0;
			tap->callback(tap->clientData, tap, chunk, &stamp, &flags, &zero, list);
			produced = got;
			if (volume == 1.0f)
				memcpy(chunkOut, tap->raw.data(), size_t(got) * l.frameBytes);
			else
			{
				float* planes[8];
				std::vector<float*> many;
				float** p = planes;
				if (l.channels > 8)
				{
					many.resize(l.channels);
					p = many.data();
				}
				for (UInt32 c = 0; c < l.channels; c++)
					p[c] = tap->plane(c);
				unpackSamples(tap->raw.data(), l, got, p, 1.0f);
				packSamples(chunkOut, l, got, p, volume);
			}
		}
		else
		{
			for (UInt32 c = 0; c < l.channels; c++)
				list->mBuffers[c] = { 1, 0, nullptr };
			UInt32 frames = 0;
			tap->callback(tap->clientData, tap, chunk, &stamp, &flags, &frames, list);
			produced = std::min(frames, chunk);
			std::vector<const float*> planes(l.channels);
			for (UInt32 c = 0; c < l.channels; c++)
				planes[c] = static_cast<const float*>(list->mBuffers[c].mData);
			packSamples(chunkOut, l, produced, planes.data(), volume);
		}
		if (produced < chunk)
			memset(chunkOut + size_t(produced) * l.frameBytes, 0, size_t(chunk - produced) * l.frameBytes);
		done += chunk;
	}
	tap->active = false;

	ioData->mBuffers[0].mDataByteSize = wantFrames * l.frameBytes;
	for (UInt32 b = 1; b < ioData->mNumberBuffers; b++)
		memset(ioData->mBuffers[b].mData, 0, ioData->mBuffers[b].mDataByteSize);

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		updateLevels(out, wantFrames);
	}

	std::vector<AudioQueueBufferRef> finished;
	finished.swap(tap->finished);
	finishRender(finished, numberFrames);
	return noErr;
}

OSStatus AudioQueueOutput::processingTapNew(AudioQueueProcessingTapCallback inCallback, void* inClientData,
		AudioQueueProcessingTapFlags inFlags, UInt32* outMaxFrames,
		AudioStreamBasicDescription* outProcessingFormat, AudioQueueProcessingTapRef* outAQTap)
{
	// One of pre- or post-effects, and optionally the siphon flag; nothing else.
	const AudioQueueProcessingTapFlags placement = inFlags & (kAudioQueueProcessingTap_PreEffects | kAudioQueueProcessingTap_PostEffects);
	if ((inFlags & ~(kAudioQueueProcessingTap_PreEffects | kAudioQueueProcessingTap_PostEffects | kAudioQueueProcessingTap_Siphon))
			|| (placement != kAudioQueueProcessingTap_PreEffects && placement != kAudioQueueProcessingTap_PostEffects))
		return kAudio_ParamError;
	SampleLayout layout;
	if (!describeSamples(m_format, layout))
		return kAudioFormatUnsupportedDataFormatError;

	std::lock_guard<std::mutex> tapLock(m_tapMutex);
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_running)
			return kAudioQueueErr_InvalidRunState;
	}
	if (m_tap)
		return kAudioQueueErr_TooManyTaps;
	m_tap = new Tap(this, layout, inCallback, inClientData, inFlags);

	*outMaxFrames = kTapMaxFrames;
	memset(outProcessingFormat, 0, sizeof(*outProcessingFormat));
	outProcessingFormat->mSampleRate = m_format.mSampleRate;
	outProcessingFormat->mFormatID = kAudioFormatLinearPCM;
	outProcessingFormat->mFormatFlags = kAudioFormatFlagsNativeFloatPacked | kAudioFormatFlagIsNonInterleaved;
	outProcessingFormat->mBytesPerPacket = 4;
	outProcessingFormat->mFramesPerPacket = 1;
	outProcessingFormat->mBytesPerFrame = 4;
	outProcessingFormat->mChannelsPerFrame = layout.channels;
	outProcessingFormat->mBitsPerChannel = 32;
	*outAQTap = m_tap;
	return noErr;
}

OSStatus AudioQueueOutput::processingTapDispose(AudioQueueProcessingTapRef inAQTap)
{
	// Synchronous: a callback in progress on another thread is waited for through m_tapMutex.
	// A callback cannot dispose its own tap.
	// QUIRK: a tap can be disposed while the queue is running, although the public header
	// asks for a stopped queue. Evidence: black-box observation on macOS 26.7.3 (tier 2),
	// AudioQueueProcessingTapDispose on a started offline-rendering queue returns noErr.
	// Test: test_quirk_dispose_tap_while_running (in test_siphon)
	Tap* current = m_tap;
	if (current && current->active && current->thread == std::this_thread::get_id())
		return kAudioQueueErr_InvalidTapContext;
	std::lock_guard<std::mutex> tapLock(m_tapMutex);
	if (inAQTap != m_tap)
		return kAudioQueueErr_InvalidParameter;
	delete m_tap;
	m_tap = nullptr;
	return noErr;
}

OSStatus AudioQueueOutput::processingTapGetSourceAudio(AudioQueueProcessingTapRef inAQTap, UInt32 inNumberFrames,
		AudioTimeStamp* ioTimeStamp, AudioQueueProcessingTapFlags* outFlags, UInt32* outNumberFrames,
		AudioBufferList* ioData)
{
	// Called from the tap's callback only. Not under m_tapMutex: the callback runs with it
	// held, by this very thread.
	Tap* tap = static_cast<Tap*>(inAQTap);
	if (!tap->active || tap->thread != std::this_thread::get_id())
		return kAudioQueueErr_InvalidTapContext;
	if (tap->flags & kAudioQueueProcessingTap_Siphon)
		return kAudioQueueErr_InvalidTapContext;
	if (!outNumberFrames)
		return kAudioQueueErr_InvalidParameter;
	OSStatus status = pullTapSource(tap, inNumberFrames, outFlags, outNumberFrames, ioData);
	if (status == noErr && ioTimeStamp)
	{
		ioTimeStamp->mFlags |= kAudioTimeStampSampleTimeValid;
		ioTimeStamp->mSampleTime = tap->chunkTime;
	}
	return status;
}

OSStatus AudioQueueOutput::pullTapSource(Tap* tap, UInt32 inNumberFrames, AudioQueueProcessingTapFlags* outFlags,
		UInt32* outNumberFrames, AudioBufferList* ioData)
{
	const SampleLayout& l = tap->layout;
	if (!ioData || ioData->mNumberBuffers != l.channels)
		return kAudioQueueErr_InvalidParameter;
	const UInt32 frames = std::min(inNumberFrames, kTapMaxFrames);
	for (UInt32 c = 0; c < l.channels; c++)
	{
		if (ioData->mBuffers[c].mData && ioData->mBuffers[c].mDataByteSize < frames * sizeof(float))
			return kAudioQueueErr_InvalidParameter;
	}

	UInt32 bytes;
	AudioQueueProcessingTapFlags flags = 0;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (tap->startOfStream)
			flags |= kAudioQueueProcessingTap_StartOfStream;
		bytes = pullLocked(tap->raw.data(), frames * l.frameBytes, tap->finished, tap->chunkTime);
		if (!m_paused && bytes < frames * l.frameBytes && m_stopWhenDrained && m_pending.empty())
			flags |= kAudioQueueProcessingTap_EndOfStream;
	}
	const UInt32 got = bytes / l.frameBytes;

	std::vector<float*> planes(l.channels);
	for (UInt32 c = 0; c < l.channels; c++)
	{
		if (!ioData->mBuffers[c].mData)
			ioData->mBuffers[c].mData = tap->plane(c);
		planes[c] = static_cast<float*>(ioData->mBuffers[c].mData);
		ioData->mBuffers[c].mNumberChannels = 1;
		ioData->mBuffers[c].mDataByteSize = got * sizeof(float);
	}
	unpackSamples(tap->raw.data(), l, got, planes.data(), 1.0f);

	if (outNumberFrames)
		*outNumberFrames = got;
	if (outFlags)
		*outFlags = flags;
	tap->sourced = true;
	return noErr;
}

OSStatus AudioQueueOutput::processingTapGetQueueTime(AudioQueueProcessingTapRef inAQTap,
		Float64* outQueueSampleTime, UInt32* outQueueFrameCount)
{
	Tap* tap = static_cast<Tap*>(inAQTap);
	// Only from the callback and after it has pulled source audio, which a siphon never does.
	if (!tap->active || tap->thread != std::this_thread::get_id() || !tap->sourced)
		return kAudioQueueErr_InvalidTapContext;
	if (!outQueueSampleTime || !outQueueFrameCount)
		return kAudioQueueErr_InvalidParameter;
	*outQueueSampleTime = tap->chunkTime;
	*outQueueFrameCount = tap->paused ? 0 : tap->chunkFrames;
	return noErr;
}

void AudioQueueOutput::returnBuffers(const std::vector<AudioQueueBufferRef>& buffers)
{
	for (AudioQueueBufferRef buffer : buffers)
	{
		AudioQueueOutputCallback callback = m_callback;
		void* userData = m_userData;
		if (m_runloop)
		{
			std::shared_ptr<CallbackGate> gate = m_gate;
			AudioQueueRef queue = this;
			CFRunLoopPerformBlock(m_runloop, m_runloopMode ? m_runloopMode : kCFRunLoopCommonModes, ^{
				std::lock_guard<std::recursive_mutex> lock(gate->mutex);
				if (gate->open)
					callback(userData, queue, buffer);
			});
			CFRunLoopWakeUp(m_runloop);
		}
		else
		{
			post([this, callback, userData, buffer] {
				callback(userData, this, buffer);
			});
		}
	}
}

bool AudioQueueOutput::onCallbackQueue() const
{
	return std::this_thread::get_id() == m_cbThread.get_id();
}

void AudioQueueOutput::drainCallbacks()
{
	// Every callback already queued runs before this returns.
	if (onCallbackQueue())
		return;
	std::unique_lock<std::mutex> lock(m_cbMutex);
	m_cbCond.wait(lock, [&] { return m_cbJobs.empty() && !m_cbBusy; });
}

OSStatus AudioQueueOutput::start(const AudioTimeStamp *inStartTime)
{
	std::lock_guard<std::mutex> control(m_controlMutex);
	bool offline;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		offline = m_offline;
	}
	OSStatus status = noErr;
	if (!offline)
		status = ensureUnit();
	if (status != noErr)
		return status;

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_paused || !m_running)
			m_streamStart = true;
		m_paused = false;
		m_lastBlockNanos = 0; // no render since the start: the position cannot be extrapolated
		m_stopWhenDrained = false;
		m_everStarted = true;
		if (m_running)
			return noErr;
	}
	if (!offline)
	{
		status = AudioOutputUnitStart(m_unit);
		if (status != noErr)
			return status;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_running = true;
	}
	notifyRunning();
	return noErr;
}

OSStatus AudioQueueOutput::prime(UInt32 inNumberOfFramesToPrepare, UInt32 *outNumberOfFramesPrepared)
{
	// Nothing is decoded ahead of time: linear PCM is played as it is.
	std::unique_lock<std::mutex> control(m_controlMutex);
	OSStatus status = noErr;
	if (!m_offline)
		status = ensureUnit();
	control.unlock();
	if (status != noErr)
		return status;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_primed = true;
	}
	if (outNumberOfFramesPrepared)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		UInt64 frames = pendingFramesLocked();
		if (inNumberOfFramesToPrepare && frames > inNumberOfFramesToPrepare)
			frames = inNumberOfFramesToPrepare;
		*outNumberOfFramesPrepared = UInt32(frames);
	}
	return noErr;
}

OSStatus AudioQueueOutput::flush()
{
	// Everything enqueued is already queued for the output unit. The time/pitch effect, which holds some audio
	// back, is told that the stream ends with what is enqueued.
	std::lock_guard<std::mutex> lock(m_mutex);
	m_flushRequested = true;
	return noErr;
}

OSStatus AudioQueueOutput::stop(Boolean inImmediate)
{
	// Released before drainCallbacks(): a deferred stop queued on the callback queue
	// takes it too.
	std::unique_lock<std::mutex> control(m_controlMutex);
	if (!inImmediate)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_running)
				return noErr;
			m_stopWhenDrained = true;
			// The time/pitch effect holds audio ahead of the output: the last of it is rendered first.
			if (!m_pending.empty() || (m_timePitch && !m_timePitch->idle()))
				return noErr;
		}
		// Nothing is waiting to be played: the unit stops once the device has played what it holds
		scheduleDrainStop();
		return noErr;
	}

	// Immediate: the unit stops now and unplayed buffers go back to the client. Without a
	// run loop no callback runs after this returns; with one, they run on that run loop. Stop returns once render() has unwound (it is
	// synchronous with the PulseAudio queue), so nothing is rendered after it.
	bool wasRunning;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		wasRunning = m_running;
		m_running = false;
	}
	if (m_unit && wasRunning)
		AudioOutputUnitStop(m_unit);
	std::vector<AudioQueueBufferRef> unplayed;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_paused = false;
		m_lastBlockNanos = 0;
		m_stopWhenDrained = false;
		m_streamStart = true;
		for (const Pending& entry : m_pending)
			unplayed.push_back(entry.buffer);
		m_pending.clear();
		m_offset = 0;
		m_gapFrames = 0;
		if (m_timePitch)
			m_timePitch->reset();
		m_mediaRendered = 0.0;
		m_flushRequested = false;
	}
	control.unlock();
	returnBuffers(unplayed);
	if (wasRunning)
		notifyRunning();
	drainCallbacks();
	m_renderedFrames.store(0, std::memory_order_relaxed);
	return noErr;
}

OSStatus AudioQueueOutput::pause()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_paused = true;
	return noErr;
}

OSStatus AudioQueueOutput::reset()
{
	std::vector<AudioQueueBufferRef> unplayed;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (const Pending& entry : m_pending)
			unplayed.push_back(entry.buffer);
		m_pending.clear();
		m_offset = 0;
		m_gapFrames = 0;
		m_stopWhenDrained = false;
		m_streamStart = true;
		m_lastBlockNanos = 0;
		m_lastBlockFrames = 0;
		m_primed = false;
		if (m_timePitch)
			m_timePitch->reset();
		m_mediaRendered = 0.0;
		m_flushRequested = false;
	}
	returnBuffers(unplayed);
	drainCallbacks();
	m_renderedFrames.store(0, std::memory_order_relaxed);
	return noErr;
}

OSStatus AudioQueueOutput::dispose(Boolean inImmediate)
{
	// Disposing always stops first, so no callback can reach a deleted queue. The queue's own
	// stop is not reported to property listeners.
	m_disposing = true;
	if (onCallbackQueue())
	{
		// Called from an output callback: finish on the callback queue, after it.
		std::lock_guard<std::mutex> control(m_controlMutex);
		bool wasRunning;
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			wasRunning = m_running;
			m_running = false;
			m_pending.clear();
		}
		if (m_unit && wasRunning)
			AudioOutputUnitStop(m_unit);
		post([this] { m_selfDelete = true; });
		return noErr;
	}
	stop(true);
	delete this;
	return noErr;
}

OSStatus AudioQueueOutput::allocateBuffer(UInt32 inBufferByteSize, UInt32 inNumberPacketDescriptions, AudioQueueBufferRef *outBuffer)
{
	if (!outBuffer)
		return kAudioQueueErr_InvalidParameter;

	AudioQueueBufferRef buffer = static_cast<AudioQueueBufferRef>(calloc(1, sizeof(AudioQueueBuffer)));
	if (!buffer)
		return memFullErr;
	buffer->mAudioData = calloc(1, inBufferByteSize ? inBufferByteSize : 1);
	if (inNumberPacketDescriptions)
		buffer->mPacketDescriptions = static_cast<AudioStreamPacketDescription*>(
			calloc(inNumberPacketDescriptions, sizeof(AudioStreamPacketDescription)));
	if (!buffer->mAudioData || (inNumberPacketDescriptions && !buffer->mPacketDescriptions))
	{
		free(buffer->mAudioData);
		free(buffer->mPacketDescriptions);
		free(buffer);
		return memFullErr;
	}
	*const_cast<UInt32*>(&buffer->mAudioDataBytesCapacity) = inBufferByteSize;
	buffer->mPacketDescriptionCapacity = inNumberPacketDescriptions;

	std::lock_guard<std::mutex> lock(m_mutex);
	m_allocated.insert(buffer);
	*outBuffer = buffer;
	return noErr;
}

OSStatus AudioQueueOutput::freeBuffer(AudioQueueBufferRef inBuffer)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_allocated.count(inBuffer))
		return kAudioQueueErr_InvalidBuffer;
	for (const Pending& pending : m_pending)
	{
		if (pending.buffer == inBuffer)
			return kAudioQueueErr_BufferInQueue;
	}
	m_allocated.erase(inBuffer);
	free(inBuffer->mAudioData);
	free(inBuffer->mPacketDescriptions);
	free(inBuffer);
	return noErr;
}

OSStatus AudioQueueOutput::enqueueBuffer(AudioQueueBufferRef inBuffer, UInt32 inNumPacketDescs, const AudioStreamPacketDescription *inPacketDescs)
{
	return enqueueBufferWithParameters(inBuffer, inNumPacketDescs, inPacketDescs, 0, 0, 0, nullptr, nullptr, nullptr);
}

OSStatus AudioQueueOutput::enqueueBufferWithParameters(AudioQueueBufferRef inBuffer, UInt32 inNumPacketDescs,
		const AudioStreamPacketDescription *inPacketDescs, UInt32 inTrimFramesAtStart, UInt32 inTrimFramesAtEnd,
		UInt32 inNumParamValues, const AudioQueueParameterEvent *inParamValues,
		const AudioTimeStamp *inStartTime, AudioTimeStamp *outActualStartTime)
{
	if (inNumParamValues && !inParamValues)
		return kAudioQueueErr_InvalidParameter;
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_allocated.count(inBuffer))
		return kAudioQueueErr_InvalidBuffer;
	if (inBuffer->mAudioDataByteSize == 0)
		return kAudioQueueErr_BufferEmpty;
	if (inBuffer->mAudioDataByteSize > inBuffer->mAudioDataBytesCapacity)
		return kAudioQueueErr_InvalidBuffer;
	for (const Pending& pending : m_pending)
	{
		if (pending.buffer == inBuffer)
			return kAudioQueueErr_BufferEnqueuedTwice;
	}

	Pending entry;
	entry.buffer = inBuffer;
	entry.startByte = 0;
	entry.endByte = inBuffer->mAudioDataByteSize;

	// QUIRK: frames to trim that together take away more than the buffer holds are not an
	// error: the trim is ignored and the whole buffer plays. Trimming exactly all of it leaves
	// nothing to play. Not documented. Evidence: black-box observation on macOS 26.7.3 x86_64
	// and 10.14.4 (tier 2). Test: test_quirk_trim_larger_than_buffer_is_ignored in
	// tests/src/audioqueue_properties.c
	const UInt64 frames = inBuffer->mAudioDataByteSize / m_format.mBytesPerFrame;
	if (UInt64(inTrimFramesAtStart) + inTrimFramesAtEnd <= frames)
	{
		entry.startByte = inTrimFramesAtStart * m_format.mBytesPerFrame;
		entry.endByte -= inTrimFramesAtEnd * m_format.mBytesPerFrame;
	}

	if (inStartTime && (inStartTime->mFlags & kAudioTimeStampSampleTimeValid))
	{
		entry.hasStart = true;
		entry.startSample = inStartTime->mSampleTime;
	}
	if (outActualStartTime)
	{
		// QUIRK: the actual start time reported back is the requested sample time (valid flag set)
		// when a start time was given, and all zeros when not, in offline rendering at least;
		// macOS 26.7.3 reports zeros in both cases. Not documented. Evidence: black-box
		// observation on macOS 10.14.4 (tier 2); the older behaviour is followed. Test:
		// test_quirk_actual_start_time_echoes_request in tests/src/audioqueue_properties.c
		memset(outActualStartTime, 0, sizeof(*outActualStartTime));
		if (entry.hasStart)
		{
			outActualStartTime->mSampleTime = entry.startSample;
			outActualStartTime->mFlags = kAudioTimeStampSampleTimeValid;
		}
	}
	entry.events.assign(inParamValues, inParamValues + inNumParamValues);
	m_pending.push_back(std::move(entry));
	return noErr;
}

double AudioQueueOutput::deviceLatencyFrames()
{
	AudioDeviceID device = kAudioObjectUnknown;
	UInt32 size = sizeof(device);
	if (!m_unit || AudioUnitGetProperty(m_unit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &device, &size) != noErr
			|| device == kAudioObjectUnknown)
		return 0;

	AudioObjectPropertyAddress address = { kAudioDevicePropertyLatency, kAudioObjectPropertyScopeOutput, kAudioObjectPropertyElementMaster };
	UInt32 frames = 0;
	size = sizeof(frames);
	if (AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &frames) != noErr)
		return 0;

	Float64 deviceRate = 0;
	address.mSelector = kAudioDevicePropertyNominalSampleRate;
	size = sizeof(deviceRate);
	if (AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &deviceRate) != noErr || deviceRate <= 0)
		return 0;
	return double(frames) * m_format.mSampleRate / deviceRate;
}

OSStatus AudioQueueOutput::getCurrentTime(AudioTimeStamp *outTimeStamp, Boolean *outTimelineDiscontinuity)
{
	if (!outTimeStamp)
		return kAudioQueueErr_InvalidParameter;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_running)
			return kAudioQueueErr_InvalidRunState;
	}
	memset(outTimeStamp, 0, sizeof(*outTimeStamp));
	// Frames heard so far: handed to the output unit, minus what it still holds. The device holds what
	// it measured when it took the last block, and that block itself; the time since then has played.
	const double rendered = double(m_renderedFrames.load(std::memory_order_relaxed));
	double heard, media;
	{
		const double deviceLatency = deviceLatencyFrames();
		std::lock_guard<std::mutex> lock(m_mutex);
		heard = rendered - std::max(deviceLatency, m_latencyFrames);
		if (deviceLatency > 0 && m_lastBlockFrames)
		{
			heard -= m_lastBlockFrames;
			if (!m_paused && m_lastBlockNanos)
				heard += double(monotonicNanos() - m_lastBlockNanos) * m_format.mSampleRate / 1e9;
			heard = std::min(heard, rendered);
		}
		// The timeline is in media time: what the time/pitch effect has stretched or squeezed counts
		// for the time it stands for, so the sample time moves at the play rate.
		if (m_mediaRendered == rendered && m_blockSpeed == 1.0)
			media = heard;
		else
			media = m_mediaRendered - (rendered - heard) * m_blockSpeed;
	}
	heard = media;
	outTimeStamp->mSampleTime = heard > 0 ? heard : 0;
	outTimeStamp->mHostTime = mach_absolute_time();
	outTimeStamp->mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid;
	if (outTimelineDiscontinuity)
		*outTimelineDiscontinuity = false;
	return noErr;
}

void AudioQueueOutput::notifyRunning()
{
	if (!m_disposing)
		notifyListeners(kAudioQueueProperty_IsRunning);
}

void AudioQueueOutput::runOnClient(std::function<void()> job)
{
	if (m_runloop)
	{
		std::shared_ptr<CallbackGate> gate = m_gate;
		CFRunLoopPerformBlock(m_runloop, m_runloopMode ? m_runloopMode : kCFRunLoopCommonModes, ^{
			std::lock_guard<std::recursive_mutex> lock(gate->mutex);
			if (gate->open)
				job();
		});
		CFRunLoopWakeUp(m_runloop);
	}
	else
		post(std::move(job));
}

// ---- what the enqueued buffers carry: start times, volume and pan ----

void AudioQueueOutput::beginEntryLocked(Pending& entry, Float64 queuePos)
{
	entry.begun = true;
	m_offset = entry.startByte;
	m_gapFrames = 0;
	const UInt32 frameBytes = m_format.mBytesPerFrame;

	if (entry.hasStart)
	{
		if (entry.startSample > queuePos)
			m_gapFrames = UInt64(std::llround(entry.startSample - queuePos));
		else
		{
			// QUIRK: a start time already in the past plays the buffer from the frame that is due:
			// the frames the start time is behind are dropped from its beginning. Not documented.
			// Evidence: black-box observation on macOS 26.7.3 x86_64 and 10.14.4 (tier 2).
			// Test: test_quirk_start_time_in_the_past_drops_frames in tests/src/audioqueue_properties.c
			UInt64 available = (entry.endByte - entry.startByte) / frameBytes;
			UInt64 cut = std::min<UInt64>(available, UInt64(std::llround(queuePos - entry.startSample)));
			m_offset += UInt32(cut * frameBytes);
		}
	}

	float rampTime = -1.0f, target = 1.0f;
	bool volume = false;
	for (const AudioQueueParameterEvent& event : entry.events)
	{
		switch (event.mID)
		{
			case kAudioQueueParam_Volume:
				target = event.mValue;
				volume = true;
				break;
			case kAudioQueueParam_VolumeRampTime:
				rampTime = event.mValue;
				break;
			case kAudioQueueParam_Pan:
				m_entryPan = std::min(1.0f, std::max(-1.0f, event.mValue));
				break;
			case kAudioQueueParam_PlayRate:
				m_playRate = event.mValue;
				break;
			case kAudioQueueParam_Pitch:
				m_pitch = event.mValue;
				break;
			default:
				break;
		}
	}
	if (!volume)
		return;
	if (queuePos <= 0 && rampTime < 0)
	{
		// The very first audio of the stream starts at its volume.
		m_entryGain = m_entryTarget = target;
		m_entryRampLeft = 0;
		return;
	}
	// QUIRK: a volume change takes effect as a linear ramp from the current volume, over the
	// ramp time of the buffer's events but never less than 25 ms (also when no ramp time is
	// given or it is zero). Not documented. Evidence: black-box observation on macOS 26.7.3
	// x86_64 and 10.14.4 (tier 2). Test: test_quirk_volume_event_ramp_minimum in
	// tests/src/audioqueue_properties.c
	const double seconds = std::max<double>(rampTime, 0.025);
	const UInt64 frames = std::max<UInt64>(1, UInt64(std::ceil(seconds * m_format.mSampleRate)));
	m_entryTarget = target;
	m_entryStep = (target - m_entryGain) / float(frames);
	m_entryRampLeft = frames;
}

UInt64 AudioQueueOutput::pendingFramesLocked() const
{
	const UInt32 frameBytes = m_format.mBytesPerFrame;
	const Float64 now = m_mediaRendered;
	Float64 pos = now;
	bool first = true;
	for (const Pending& entry : m_pending)
	{
		UInt64 frames;
		if (first && entry.begun)
		{
			pos += Float64(m_gapFrames);
			frames = (entry.endByte - m_offset) / frameBytes;
		}
		else
		{
			frames = (entry.endByte - entry.startByte) / frameBytes;
			if (entry.hasStart)
			{
				if (entry.startSample > pos)
					pos = entry.startSample;
				else
					frames -= std::min<UInt64>(frames, UInt64(std::llround(pos - entry.startSample)));
			}
		}
		first = false;
		pos += Float64(frames);
	}
	return UInt64(pos - now);
}

namespace
{
	// Scales every frame of interleaved (or mono) PCM: the volume starts at `gain` and moves by
	// `step` per frame; pan lowers the left channel for positive and the right for negative values.
	void scaleFrames(const AudioStreamBasicDescription& format, uint8_t* data, UInt32 frames, float gain, float step, float pan)
	{
		SampleLayout l;
		if (!describeSamples(format, l))
			return;
		const float left = pan > 0 ? 1.0f - pan : 1.0f;
		const float right = pan < 0 ? 1.0f + pan : 1.0f;
		for (UInt32 i = 0; i < frames; i++)
		{
			const float g = gain + step * float(i);
			uint8_t* frame = data + size_t(i) * l.frameBytes;
			for (UInt32 c = 0; c < l.channels; c++)
			{
				float factor = g;
				if (l.channels >= 2 && c == 0)
					factor *= left;
				else if (l.channels >= 2 && c == 1)
					factor *= right;
				uint8_t* sample = frame + c * l.chunkStep;
				storeSample(sample, l, loadSample(sample, l) * factor);
			}
		}
	}
}

void AudioQueueOutput::applyEntryGain(uint8_t* data, UInt32 frames)
{
	if (!frames || (!m_entryRampLeft && m_entryGain == 1.0f && m_entryPan == 0.0f))
		return;
	UInt32 done = 0;
	while (done < frames)
	{
		// A ramp is applied in one run, the rest of the frames at the final volume.
		UInt32 run = m_entryRampLeft ? UInt32(std::min<UInt64>(m_entryRampLeft, frames - done)) : frames - done;
		scaleFrames(m_format, data + size_t(done) * m_format.mBytesPerFrame, run, m_entryGain, m_entryRampLeft ? m_entryStep : 0.0f, m_entryPan);
		if (m_entryRampLeft)
		{
			m_entryGain += m_entryStep * float(run);
			m_entryRampLeft -= run;
			if (!m_entryRampLeft)
				m_entryGain = m_entryTarget;
		}
		done += run;
	}
}

void AudioQueueOutput::applyQueueGain(uint8_t* data, UInt32 frames)
{
	const float volume = m_volume.load(std::memory_order_relaxed);
	const float pan = std::min(1.0f, std::max(-1.0f, m_pan.load(std::memory_order_relaxed)));
	if (frames && (volume != 1.0f || pan != 0.0f))
		scaleFrames(m_format, data, frames, volume, 0.0f, pan);
}

void AudioQueueOutput::updateLevels(const uint8_t* data, UInt32 frames)
{
	if (!m_metering || !frames)
		return;
	SampleLayout l;
	if (!describeSamples(m_format, l))
		return;
	std::vector<double> sums(l.channels, 0.0);
	std::vector<float> peaks(l.channels, 0.0f);
	for (UInt32 i = 0; i < frames; i++)
	{
		for (UInt32 c = 0; c < l.channels; c++)
		{
			const float v = loadSample(data + size_t(i) * l.frameBytes + c * l.chunkStep, l);
			peaks[c] = std::max(peaks[c], std::fabs(v));
			sums[c] += double(v) * v;
		}
	}
	m_levels.resize(l.channels);
	for (UInt32 c = 0; c < l.channels; c++)
		m_levels[c] = AudioQueueLevelMeterState { float(std::sqrt(sums[c] / frames)), peaks[c] };
}

// ---- parameters and properties ----

OSStatus AudioQueueOutput::getParameter(AudioQueueParameterID inParamID, AudioQueueParameterValue *outValue)
{
	if (!outValue)
		return kAudioQueueErr_InvalidParameter;
	switch (inParamID)
	{
		case kAudioQueueParam_Volume:
			*outValue = m_volume.load(std::memory_order_relaxed);
			return noErr;
		case kAudioQueueParam_Pan:
			*outValue = m_pan.load(std::memory_order_relaxed);
			return noErr;
		case kAudioQueueParam_PlayRate:
		case kAudioQueueParam_Pitch:
		case kAudioQueueParam_VolumeRampTime:
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			*outValue = inParamID == kAudioQueueParam_PlayRate ? m_playRate
					: inParamID == kAudioQueueParam_Pitch ? m_pitch : m_volumeRampTime;
			return noErr;
		}
	}
	return kAudioQueueErr_InvalidParameter;
}

OSStatus AudioQueueOutput::setParameter(AudioQueueParameterID inParamID, AudioQueueParameterValue inValue)
{
	switch (inParamID)
	{
		case kAudioQueueParam_Volume:
			m_volume.store(inValue, std::memory_order_relaxed);
			return noErr;
		case kAudioQueueParam_Pan:
			m_pan.store(inValue, std::memory_order_relaxed);
			return noErr;
		case kAudioQueueParam_PlayRate:
		case kAudioQueueParam_Pitch:
		case kAudioQueueParam_VolumeRampTime:
		{
			// QUIRK: any play rate and pitch is accepted without an error and reads back as set, also outside
			// the documented range of 0.5 to 2. The time/pitch effect acts on rates from 1/32 to 32
			// and on a pitch of up to 2400 cents either way (more is cut to that), and it is applied with
			// the next block that is rendered.
			// Not documented. Evidence: black-box observation on macOS 26.7.3 x86_64 and 10.14.4 i386
			// (tier 2): rates 0.25, 3 and 4 with TimeDomain play at that rate, a pitch of +-3000 cents with
			// Spectral sounds as +-2400.
			// Test: test_play_rate_keeps_the_pitch and test_pitch in tests/src/audioqueue_timepitch.c
			std::lock_guard<std::mutex> lock(m_mutex);
			if (inParamID == kAudioQueueParam_PlayRate)
				m_playRate = inValue;
			else if (inParamID == kAudioQueueParam_Pitch)
				m_pitch = inValue;
			else
				m_volumeRampTime = inValue;
			return noErr;
		}
	}
	return kAudioQueueErr_InvalidParameter;
}

AudioDeviceID AudioQueueOutput::resolveDevice()
{
	AudioDeviceID device = kAudioObjectUnknown;
	CFStringRef uid;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		uid = m_deviceUID ? (CFStringRef) CFRetain(m_deviceUID) : nullptr;
	}
	if (uid)
	{
		AudioValueTranslation translation = { &uid, sizeof(uid), &device, sizeof(device) };
		AudioObjectPropertyAddress address = { kAudioHardwarePropertyDeviceForUID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
		UInt32 size = sizeof(translation);
		if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &size, &translation) != noErr)
			device = kAudioObjectUnknown;
		CFRelease(uid);
	}
	else
	{
		AudioObjectPropertyAddress address = { kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
		UInt32 size = sizeof(device);
		if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &size, &device) != noErr)
			device = kAudioObjectUnknown;
	}
	if (device == kAudioObjectUnknown)
		return device;

	// It must be able to play.
	AudioObjectPropertyAddress streams = { kAudioDevicePropertyStreams, kAudioObjectPropertyScopeOutput, kAudioObjectPropertyElementMaster };
	UInt32 size = 0;
	if (AudioObjectGetPropertyDataSize(device, &streams, 0, nullptr, &size) != noErr || size == 0)
		return kAudioObjectUnknown;
	return device;
}

OSStatus AudioQueueOutput::getPropertySize(AudioQueuePropertyID inID, UInt32 *outDataSize)
{
	if (!outDataSize)
		return kAudioQueueErr_InvalidParameter;
	switch (inID)
	{
		case kAudioQueueProperty_IsRunning:
		case kAudioQueueProperty_EnableLevelMetering:
		case kAudioQueueProperty_DecodeBufferSizeFrames:
		case kAudioQueueProperty_ConverterError:
		case kAudioQueueProperty_EnableTimePitch:
		case kAudioQueueProperty_TimePitchAlgorithm:
		case kAudioQueueProperty_TimePitchBypass:
		case kAudioQueueDeviceProperty_NumberChannels:
			*outDataSize = sizeof(UInt32);
			return noErr;
		case kAudioQueueDeviceProperty_SampleRate:
			*outDataSize = sizeof(Float64);
			return noErr;
		case kAudioQueueProperty_CurrentDevice:
			*outDataSize = sizeof(CFStringRef);
			return noErr;
		case kAudioQueueProperty_StreamDescription:
			*outDataSize = sizeof(AudioStreamBasicDescription);
			return noErr;
		case kAudioQueueProperty_CurrentLevelMeter:
		case kAudioQueueProperty_CurrentLevelMeterDB:
			*outDataSize = m_format.mChannelsPerFrame * sizeof(AudioQueueLevelMeterState);
			return noErr;
		case kAudioQueueProperty_ChannelLayout:
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_channelLayout.empty())
				return kAudioQueueErr_InvalidProperty;
			*outDataSize = UInt32(m_channelLayout.size());
			return noErr;
		}
	}
	return kAudioQueueErr_InvalidProperty;
}

OSStatus AudioQueueOutput::getProperty(AudioQueuePropertyID inID, void *outData, UInt32 *ioDataSize)
{
	if (!outData || !ioDataSize)
		return kAudioQueueErr_InvalidParameter;
	UInt32 need;
	OSStatus status = getPropertySize(inID, &need);
	if (status != noErr)
		return status;
	// The running flag is read with a buffer of exactly four bytes (the same rule as an input queue's).
	if (inID == kAudioQueueProperty_IsRunning ? *ioDataSize != need : *ioDataSize < need)
		return kAudioQueueErr_InvalidPropertySize;

	switch (inID)
	{
		case kAudioQueueProperty_IsRunning:
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			*static_cast<UInt32*>(outData) = m_running ? 1 : 0;
			break;
		}
		case kAudioQueueProperty_StreamDescription:
			memcpy(outData, &m_format, sizeof(m_format));
			break;
		case kAudioQueueProperty_ConverterError:
			*static_cast<UInt32*>(outData) = 0;
			break;
		case kAudioQueueProperty_EnableLevelMetering:
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			*static_cast<UInt32*>(outData) = m_metering ? 1 : 0;
			break;
		}
		case kAudioQueueProperty_CurrentLevelMeter:
		case kAudioQueueProperty_CurrentLevelMeterDB:
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_metering)
				return kAudioQueueErr_InvalidPropertyValue;
			AudioQueueLevelMeterState* out = static_cast<AudioQueueLevelMeterState*>(outData);
			for (UInt32 c = 0; c < m_format.mChannelsPerFrame; c++)
			{
				out[c] = c < m_levels.size() ? m_levels[c] : AudioQueueLevelMeterState { 0, 0 };
				if (inID == kAudioQueueProperty_CurrentLevelMeterDB)
				{
					// Decibels relative to full scale; silence is reported as -160 dB.
					out[c].mAveragePower = 20 * std::log10(std::max(out[c].mAveragePower, 1e-8f));
					out[c].mPeakPower = 20 * std::log10(std::max(out[c].mPeakPower, 1e-8f));
				}
			}
			break;
		}
		case kAudioQueueProperty_DecodeBufferSizeFrames:
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			// Without a setting, the queue decodes ahead by a second and a half.
			*static_cast<UInt32*>(outData) = m_decodeBufferFrames ? m_decodeBufferFrames : UInt32(m_format.mSampleRate * 1.5);
			break;
		}
		case kAudioQueueProperty_EnableTimePitch:
			*static_cast<UInt32*>(outData) = m_timePitchEnabled;
			break;
		case kAudioQueueProperty_TimePitchAlgorithm:
			*static_cast<UInt32*>(outData) = m_timePitchAlgorithm;
			break;
		case kAudioQueueProperty_TimePitchBypass:
			*static_cast<UInt32*>(outData) = m_timePitchBypass;
			break;
		case kAudioQueueProperty_ChannelLayout:
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_channelLayout.size() != need)
				return kAudioQueueErr_InvalidPropertySize;
			memcpy(outData, m_channelLayout.data(), need);
			break;
		}
		case kAudioQueueProperty_CurrentDevice:
		{
			CFStringRef uid = nullptr;
			status = deviceUID(&uid);
			if (status != noErr)
				return status;
			*static_cast<CFStringRef*>(outData) = uid; // the caller releases it
			break;
		}
		case kAudioQueueDeviceProperty_SampleRate:
		case kAudioQueueDeviceProperty_NumberChannels:
		{
			AudioDeviceID device = resolveDevice();
			if (device == kAudioObjectUnknown)
				return kAudioQueueErr_InvalidDevice;
			if (inID == kAudioQueueDeviceProperty_SampleRate)
			{
				Float64 rate = 0;
				UInt32 size = sizeof(rate);
				AudioObjectPropertyAddress address = { kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeOutput, kAudioObjectPropertyElementMaster };
				if (AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &rate) != noErr || rate <= 0)
					return kAudioQueueErr_InvalidDevice;
				*static_cast<Float64*>(outData) = rate;
			}
			else
			{
				AudioObjectPropertyAddress address = { kAudioDevicePropertyStreamConfiguration, kAudioObjectPropertyScopeOutput, kAudioObjectPropertyElementMaster };
				UInt32 size = 0;
				if (AudioObjectGetPropertyDataSize(device, &address, 0, nullptr, &size) != noErr || size < sizeof(AudioBufferList))
					return kAudioQueueErr_InvalidDevice;
				std::vector<uint8_t> memory(size);
				AudioBufferList* list = reinterpret_cast<AudioBufferList*>(memory.data());
				if (AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, list) != noErr)
					return kAudioQueueErr_InvalidDevice;
				UInt32 channels = 0;
				for (UInt32 i = 0; i < list->mNumberBuffers; i++)
					channels += list->mBuffers[i].mNumberChannels;
				*static_cast<UInt32*>(outData) = channels;
			}
			break;
		}
	}
	*ioDataSize = need;
	return noErr;
}

OSStatus AudioQueueOutput::deviceUID(CFStringRef* outUID)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_deviceUID)
		{
			*outUID = (CFStringRef) CFRetain(m_deviceUID);
			return noErr;
		}
	}
	// No device chosen: the one the queue would play to.
	AudioDeviceID device = resolveDevice();
	CFStringRef uid = nullptr;
	if (device != kAudioObjectUnknown)
	{
		UInt32 size = sizeof(uid);
		AudioObjectPropertyAddress address = { kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
		if (AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &uid) != noErr)
			uid = nullptr;
	}
	if (!uid)
	{
		// QUIRK: a queue that has not been given a device and has no default output device
		// to ask reports a placeholder UID: "AQDefaultOutput" on macOS 10.14.4 and
		// "AQDefaultDevice" on 26.7.3 (both 15 characters, followed here is the older one).
		// Not documented. Evidence: black-box observation on both, which had no audio
		// device (tier 2). Test: test_quirk_default_device_uid_placeholder in
		// tests/src/audioqueue_properties.c
		uid = CFStringCreateCopy(nullptr, CFSTR("AQDefaultOutput"));
	}
	*outUID = uid;
	return noErr;
}

OSStatus AudioQueueOutput::setProperty(AudioQueuePropertyID inID, const void *inData, UInt32 inDataSize)
{
	if (!inData)
		return kAudio_ParamError;
	switch (inID)
	{
		case kAudioQueueProperty_EnableLevelMetering:
		{
			if (inDataSize != sizeof(UInt32))
				return kAudioQueueErr_InvalidPropertySize;
			std::lock_guard<std::mutex> lock(m_mutex);
			const bool enable = *static_cast<const UInt32*>(inData) != 0;
			if (enable && !m_metering)
				m_levels.assign(m_format.mChannelsPerFrame, AudioQueueLevelMeterState { 0, 0 });
			m_metering = enable;
			return noErr;
		}
		case kAudioQueueProperty_CurrentDevice:
		{
			if (inDataSize != sizeof(CFStringRef))
				return kAudioQueueErr_InvalidPropertySize;
			CFStringRef uid = *static_cast<const CFStringRef*>(inData);
			if (uid && CFGetTypeID(uid) != CFStringGetTypeID())
				return kAudioQueueErr_InvalidPropertyValue;
			// The device is changed while the queue is stopped; the next start plays to it. An
			// unknown UID is reported when the queue is started. No string means the default device.
			std::lock_guard<std::mutex> control(m_controlMutex);
			CFStringRef old;
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				if (m_running)
					return kAudioQueueErr_InvalidRunState;
				// A queue that renders offline has no device to choose.
				if (m_offline)
					return kAudioQueueErr_InvalidOfflineMode;
				old = m_deviceUID;
				m_deviceUID = uid ? (CFStringRef) CFRetain(uid) : nullptr;
			}
			if (old)
				CFRelease(old);
			if (m_unit)
			{
				AudioUnitUninitialize(m_unit);
				AudioComponentInstanceDispose(m_unit);
				m_unit = nullptr;
			}
			return noErr;
		}
		case kAudioQueueProperty_DecodeBufferSizeFrames:
		{
			if (inDataSize != sizeof(UInt32))
				return kAudioQueueErr_InvalidPropertySize;
			std::lock_guard<std::mutex> lock(m_mutex);
			m_decodeBufferFrames = *static_cast<const UInt32*>(inData);
			return noErr;
		}
		case kAudioQueueProperty_EnableTimePitch:
		case kAudioQueueProperty_TimePitchAlgorithm:
		case kAudioQueueProperty_TimePitchBypass:
		{
			if (inDataSize != sizeof(UInt32))
				return kAudioQueueErr_InvalidPropertySize;
			const UInt32 value = *static_cast<const UInt32*>(inData);
			std::lock_guard<std::mutex> lock(m_mutex);
			// QUIRK: the time/pitch properties are checked as follows, with status codes that
			// the headers do not name (kTimePitchNotAllowed, kTimePitchInvalidValue):
			//  - EnableTimePitch: kAudioQueueErr_InvalidRunState while the queue runs;
			//    kTimePitchNotAllowed when it would switch the effect on after AudioQueuePrime
			//    (AudioQueueReset allows it again); otherwise any value is accepted, nonzero
			//    reads back as 1. Starting and stopping a queue does not lock the setting.
			//  - TimePitchAlgorithm: kAudioQueueErr_InvalidRunState while the queue runs;
			//    kTimePitchInvalidValue when the effect is not enabled or the value is not one of
			//    the three algorithms; kTimePitchNotAllowed for the deprecated
			//    low-quality-zero-latency algorithm (not offered by the queue). It may be changed
			//    after AudioQueuePrime.
			//  - TimePitchBypass: may be changed while the queue runs, but only on a queue that
			//    has been started at least once and has the effect enabled, else
			//    kTimePitchInvalidValue. Any nonzero value bypasses.
			// Not documented. Evidence: black-box observation on macOS 26.7.3 x86_64 and 10.14.4
			// i386 (tier 2), on queues in offline render mode (the Macs used had no audio output
			// device, and on a queue bound to a device that does not exist every one of these
			// returns kAudioQueueErr_InvalidDevice).
			// Test: test_quirk_timepitch_property_status in tests/src/audioqueue_timepitch.c
			if (inID == kAudioQueueProperty_EnableTimePitch)
			{
				if (m_running)
					return kAudioQueueErr_InvalidRunState;
				const bool enable = value != 0;
				if (enable && !m_timePitchEnabled && m_primed)
					return kTimePitchNotAllowed;
				m_timePitchEnabled = enable;
			}
			else if (inID == kAudioQueueProperty_TimePitchAlgorithm)
			{
				if (m_running)
					return kAudioQueueErr_InvalidRunState;
				if (!m_timePitchEnabled)
					return kTimePitchInvalidValue;
				if (value == kAudioQueueTimePitchAlgorithm_LowQualityZeroLatency)
					return kTimePitchNotAllowed;
				if (value != kAudioQueueTimePitchAlgorithm_Spectral && value != kAudioQueueTimePitchAlgorithm_TimeDomain
						&& value != kAudioQueueTimePitchAlgorithm_Varispeed)
					return kTimePitchInvalidValue;
				m_timePitchAlgorithm = value;
			}
			else
			{
				if (!m_timePitchEnabled || !m_everStarted)
					return kTimePitchInvalidValue;
				m_timePitchBypass = value != 0;
			}
			return noErr;
		}
		case kAudioQueueProperty_ChannelLayout:
		{
			if (inDataSize < offsetof(AudioChannelLayout, mChannelDescriptions))
				return kAudioQueueErr_InvalidPropertySize;
			std::lock_guard<std::mutex> lock(m_mutex);
			m_channelLayout.assign(static_cast<const uint8_t*>(inData), static_cast<const uint8_t*>(inData) + inDataSize);
			return noErr;
		}
		case kAudioQueueProperty_IsRunning:
		case kAudioQueueProperty_StreamDescription:
		case kAudioQueueProperty_CurrentLevelMeter:
		case kAudioQueueProperty_CurrentLevelMeterDB:
		case kAudioQueueProperty_ConverterError:
		case kAudioQueueDeviceProperty_SampleRate:
		case kAudioQueueDeviceProperty_NumberChannels:
			return kAudioQueueErr_InvalidProperty; // read-only
	}
	return kAudioQueueErr_InvalidProperty;
}

OSStatus AudioQueueOutput::setOfflineRenderFormat(const AudioStreamBasicDescription *inFormat, const AudioChannelLayout *inLayout)
{
	// A NULL format leaves offline mode. Either way the queue must be stopped.
	std::lock_guard<std::mutex> control(m_controlMutex);
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_running)
		return kAudioQueueErr_InvalidRunState;
	if (!inFormat)
	{
		m_offline = false;
		return noErr;
	}
	// Linear PCM at the queue's sample rate and channel count; the sample format may differ.
	SampleLayout layout;
	if (!describeSamples(*inFormat, layout))
		return kAudioFormatUnsupportedDataFormatError;
	if (inFormat->mSampleRate != m_format.mSampleRate || inFormat->mChannelsPerFrame != m_format.mChannelsPerFrame)
		return kAudioFormatUnsupportedDataFormatError;
	m_offlineFormat = *inFormat;
	m_offline = true;
	return noErr;
}

OSStatus AudioQueueOutput::offlineRender(const AudioTimeStamp *inTimestamp, AudioQueueBufferRef ioBuffer, UInt32 inNumberFrames)
{
	SampleLayout queueLayout, offlineLayout;
	AudioStreamBasicDescription offlineFormat;
	UInt32 frames;
	bool effect;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_offline)
			return kAudioQueueErr_InvalidOfflineMode;
		if (!m_running)
			return kAudioQueueErr_InvalidRunState;
		if (!inTimestamp || !(inTimestamp->mFlags & kAudioTimeStampSampleTimeValid) || !ioBuffer || !inNumberFrames)
			return kAudio_ParamError;
		if (!m_allocated.count(ioBuffer))
			return kAudioQueueErr_InvalidBuffer;
		offlineFormat = m_offlineFormat;
		describeSamples(m_format, queueLayout);
		describeSamples(offlineFormat, offlineLayout);
		if (UInt64(inNumberFrames) * offlineLayout.frameBytes > ioBuffer->mAudioDataBytesCapacity)
			return kAudio_ParamError;

		// Only what has been enqueued is rendered: the buffer's size says how much that was
		// (and a tap is asked for that many frames, not for the full count).
		// With the time/pitch effect on, the number of frames the enqueued audio makes is not known
		// beforehand: the block is rendered in full, and what carries audio is reported.
		effect = timePitchInUseLocked();
		frames = UInt32(std::min<UInt64>(pendingFramesLocked(), inNumberFrames));
		if (effect)
			frames = pendingFramesLocked() || (m_timePitch && !m_timePitch->idle()) ? inNumberFrames : 0;
	}
	if (!frames)
	{
		ioBuffer->mAudioDataByteSize = 0;
		return noErr;
	}

	// Render in the queue's format, then convert to the offline format when it differs.
	std::vector<uint8_t> queueData(size_t(frames) * queueLayout.frameBytes);
	AudioBufferList list;
	list.mNumberBuffers = 1;
	list.mBuffers[0].mNumberChannels = queueLayout.channels;
	list.mBuffers[0].mDataByteSize = UInt32(queueData.size());
	list.mBuffers[0].mData = queueData.data();
	UInt32 audioFrames = 0;
	render(frames, &list, &audioFrames);
	const UInt32 producedFrames = effect ? audioFrames : list.mBuffers[0].mDataByteSize / queueLayout.frameBytes;
	if (effect && producedFrames < frames)
	{
		// The end of the stream: the rest of the block is not part of the timeline.
		std::lock_guard<std::mutex> lock(m_mutex);
		m_renderedFrames.fetch_sub(frames - producedFrames, std::memory_order_relaxed);
		m_mediaRendered -= double(frames - producedFrames) * m_blockSpeed;
	}

	uint8_t* out = static_cast<uint8_t*>(ioBuffer->mAudioData);
	if (offlineFormat.mFormatFlags == m_format.mFormatFlags && offlineFormat.mBitsPerChannel == m_format.mBitsPerChannel)
		memcpy(out, queueData.data(), size_t(producedFrames) * queueLayout.frameBytes);
	else
	{
		std::vector<float> samples(size_t(frames) * queueLayout.channels);
		std::vector<float*> planes(queueLayout.channels);
		for (UInt32 c = 0; c < queueLayout.channels; c++)
			planes[c] = samples.data() + size_t(c) * frames;
		unpackSamples(queueData.data(), queueLayout, producedFrames, planes.data(), 1.0f);
		packSamples(out, offlineLayout, producedFrames, planes.data(), 1.0f);
	}
	ioBuffer->mAudioDataByteSize = producedFrames * offlineLayout.frameBytes;
	return noErr;
}
