#include "AudioQueueOutput.h"
#include "stub.h"
#include <CarbonCore/MacErrors.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioFormat.h>
#include <CoreAudio/AudioHardware.h>
#include <mach/mach_time.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>

static const UInt32 kDeviceBufferFrames = 2048;

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

	AudioComponentDescription desc = { kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponent component = AudioComponentFindNext(nullptr, &desc);
	if (!component)
		return kAudioQueueErr_InvalidDevice;

	AudioComponentInstance unit;
	OSStatus status = AudioComponentInstanceNew(component, &unit);
	if (status != noErr)
		return status;

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

UInt32 AudioQueueOutput::pullLocked(uint8_t* out, UInt32 want, std::vector<AudioQueueBufferRef>& finished)
{
	UInt32 done = 0;
	while (!m_paused && done < want && !m_pending.empty())
	{
		AudioQueueBufferRef buffer = m_pending.front();
		UInt32 left = buffer->mAudioDataByteSize - m_offset;
		UInt32 n = std::min(left, want - done);
		memcpy(out + done, static_cast<uint8_t*>(buffer->mAudioData) + m_offset, n);
		done += n;
		m_offset += n;
		if (m_offset >= buffer->mAudioDataByteSize)
		{
			m_pending.pop_front();
			m_offset = 0;
			finished.push_back(buffer);
		}
	}
	return done;
}

OSStatus AudioQueueOutput::render(UInt32 numberFrames, AudioBufferList* ioData)
{
	{
		std::lock_guard<std::mutex> tapLock(m_tapMutex);
		if (m_tap)
			return renderTapped(numberFrames, ioData);
	}

	std::vector<AudioQueueBufferRef> finished;
	const UInt32 frameBytes = m_format.mBytesPerFrame;
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		for (UInt32 b = 0; b < ioData->mNumberBuffers; b++)
		{
			uint8_t* out = static_cast<uint8_t*>(ioData->mBuffers[b].mData);
			UInt32 want = std::min<UInt32>(ioData->mBuffers[b].mDataByteSize, numberFrames * frameBytes);
			UInt32 done = pullLocked(out, want, finished);
			// Underrun or pause: silence (zero is silence for float and signed PCM).
			if (done < want)
				memset(out + done, 0, want - done);
			ioData->mBuffers[b].mDataByteSize = want;

			const float volume = m_volume.load(std::memory_order_relaxed);
			if (volume != 1.0f && (m_format.mFormatFlags & kAudioFormatFlagIsFloat) && m_format.mBitsPerChannel == 32)
			{
				float* samples = reinterpret_cast<float*>(out);
				for (UInt32 i = 0; i < done / sizeof(float); i++)
					samples[i] *= volume;
			}
		}
	}
	finishRender(finished, numberFrames);
	return noErr;
}

void AudioQueueOutput::finishRender(std::vector<AudioQueueBufferRef>& finished, UInt32 numberFrames)
{
	bool stopNow;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_paused)
			m_renderedFrames.fetch_add(numberFrames, std::memory_order_relaxed);
		stopNow = m_stopWhenDrained && m_pending.empty();
	}

	returnBuffers(finished);

	if (stopNow)
	{
		// A deferred AudioQueueStop: everything enqueued has played. Stopping the unit
		// from its own render thread is not allowed, so it happens on the callback queue.
		post([this] {
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
		});
	}
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
		bytes = pullLocked(tap->raw.data(), frames * l.frameBytes, tap->finished);
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
		m_stopWhenDrained = false;
		if (m_running)
			return noErr;
	}
	if (!offline)
	{
		status = AudioOutputUnitStart(m_unit);
		if (status != noErr)
			return status;
	}
	std::lock_guard<std::mutex> lock(m_mutex);
	m_running = true;
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
	if (outNumberOfFramesPrepared)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		UInt64 bytes = 0;
		for (AudioQueueBufferRef buffer : m_pending)
			bytes += buffer->mAudioDataByteSize;
		bytes -= m_offset;
		UInt64 frames = bytes / m_format.mBytesPerFrame;
		if (inNumberOfFramesToPrepare && frames > inNumberOfFramesToPrepare)
			frames = inNumberOfFramesToPrepare;
		*outNumberOfFramesPrepared = UInt32(frames);
	}
	return noErr;
}

OSStatus AudioQueueOutput::flush()
{
	// Everything enqueued is already queued for the output unit.
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
			if (!m_pending.empty())
			{
				m_stopWhenDrained = true;
				return noErr;
			}
			m_running = false;
		}
		if (m_unit)
			AudioOutputUnitStop(m_unit);
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
		m_stopWhenDrained = false;
		m_streamStart = true;
		unplayed.assign(m_pending.begin(), m_pending.end());
		m_pending.clear();
		m_offset = 0;
	}
	control.unlock();
	returnBuffers(unplayed);
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
		unplayed.assign(m_pending.begin(), m_pending.end());
		m_pending.clear();
		m_offset = 0;
		m_stopWhenDrained = false;
		m_streamStart = true;
	}
	returnBuffers(unplayed);
	drainCallbacks();
	m_renderedFrames.store(0, std::memory_order_relaxed);
	return noErr;
}

OSStatus AudioQueueOutput::dispose(Boolean inImmediate)
{
	// Disposing always stops first, so no callback can reach a deleted queue.
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
	for (AudioQueueBufferRef pending : m_pending)
	{
		if (pending == inBuffer)
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
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_allocated.count(inBuffer))
		return kAudioQueueErr_InvalidBuffer;
	if (inBuffer->mAudioDataByteSize == 0)
		return kAudioQueueErr_BufferEmpty;
	if (inBuffer->mAudioDataByteSize > inBuffer->mAudioDataBytesCapacity)
		return kAudioQueueErr_InvalidBuffer;
	for (AudioQueueBufferRef pending : m_pending)
	{
		if (pending == inBuffer)
			return kAudioQueueErr_BufferEnqueuedTwice;
	}
	m_pending.push_back(inBuffer);
	return noErr;
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
	// Frames heard so far: handed to the output unit, minus what it still holds.
	double heard = double(m_renderedFrames.load(std::memory_order_relaxed)) - m_latencyFrames;
	outTimeStamp->mSampleTime = heard > 0 ? heard : 0;
	outTimeStamp->mHostTime = mach_absolute_time();
	outTimeStamp->mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid;
	if (outTimelineDiscontinuity)
		*outTimelineDiscontinuity = false;
	return noErr;
}

OSStatus AudioQueueOutput::getParameter(AudioQueueParameterID inParamID, AudioQueueParameterValue *outValue)
{
	if (inParamID != kAudioQueueParam_Volume || !outValue)
		return kAudioQueueErr_InvalidParameter;
	*outValue = m_volume.load(std::memory_order_relaxed);
	return noErr;
}

OSStatus AudioQueueOutput::setParameter(AudioQueueParameterID inParamID, AudioQueueParameterValue inValue)
{
	if (inParamID != kAudioQueueParam_Volume || !(inValue >= 0.0f && inValue <= 1.0f))
		return kAudioQueueErr_InvalidParameter;
	m_volume.store(inValue, std::memory_order_relaxed);
	return noErr;
}

OSStatus AudioQueueOutput::getPropertySize(AudioQueuePropertyID inID, UInt32 *outDataSize)
{
	if (!outDataSize)
		return kAudioQueueErr_InvalidParameter;
	switch (inID)
	{
		case kAudioQueueProperty_IsRunning:
			*outDataSize = sizeof(UInt32);
			return noErr;
		case kAudioQueueProperty_StreamDescription:
			*outDataSize = sizeof(AudioStreamBasicDescription);
			return noErr;
	}
	return kAudioQueueErr_InvalidProperty;
}

OSStatus AudioQueueOutput::getProperty(AudioQueuePropertyID inID, void *outData, UInt32 *ioDataSize)
{
	if (!outData || !ioDataSize)
		return kAudioQueueErr_InvalidParameter;
	switch (inID)
	{
		case kAudioQueueProperty_IsRunning:
		{
			if (*ioDataSize < sizeof(UInt32))
				return kAudioQueueErr_InvalidPropertySize;
			std::lock_guard<std::mutex> lock(m_mutex);
			*static_cast<UInt32*>(outData) = m_running ? 1 : 0;
			*ioDataSize = sizeof(UInt32);
			return noErr;
		}
		case kAudioQueueProperty_StreamDescription:
			if (*ioDataSize < sizeof(AudioStreamBasicDescription))
				return kAudioQueueErr_InvalidPropertySize;
			memcpy(outData, &m_format, sizeof(m_format));
			*ioDataSize = sizeof(AudioStreamBasicDescription);
			return noErr;
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
		UInt64 bytes = 0;
		for (AudioQueueBufferRef buffer : m_pending)
			bytes += buffer->mAudioDataByteSize;
		bytes -= m_offset;
		frames = UInt32(std::min<UInt64>(bytes / queueLayout.frameBytes, inNumberFrames));
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
	render(frames, &list);
	const UInt32 producedFrames = list.mBuffers[0].mDataByteSize / queueLayout.frameBytes;

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
