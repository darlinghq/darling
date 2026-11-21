// Copyright (C) 2026 Lubos Dolezel
#include "AudioQueueInput.h"
#include <Block.h>
#include <CarbonCore/MacErrors.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioFormat.h>
#include <mach/mach_time.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

static const UInt32 kInputBus = 1;
static const UInt32 kOutputBus = 0;

AudioQueueInput::AudioQueueInput(const AudioStreamBasicDescription *inFormat, AudioQueueInputCallback inCallbackProc,
		void *inUserData, CFRunLoopRef inCallbackRunLoop, CFStringRef inCallbackRunLoopMode,
		dispatch_queue_t inCallbackQueue, AudioQueueInputCallbackBlock inCallbackBlock)
: OpaqueAudioQueue(inFormat, inUserData, inCallbackRunLoop, inCallbackRunLoopMode, 0), m_callback(inCallbackProc)
{
	if (inCallbackBlock)
		m_block = Block_copy(inCallbackBlock);
	if (inCallbackQueue)
	{
		m_dispatchQueue = inCallbackQueue;
		dispatch_retain(m_dispatchQueue);
	}
	m_levels.assign(inFormat->mChannelsPerFrame, AudioQueueLevelMeterState { 0, 0 });
	m_cbThread = std::thread(&AudioQueueInput::callbackThread, this);
}

AudioQueueInput::~AudioQueueInput()
{
	{
		std::lock_guard<std::recursive_mutex> lock(m_gate->mutex);
		m_gate->open = false;
	}
	destroyUnit();
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
	if (m_converter)
		AudioConverterDispose(m_converter);
	for (AudioQueueBufferRef buffer : m_allocated)
	{
		free(buffer->mAudioData);
		free(buffer->mPacketDescriptions);
		free(buffer);
	}
	if (m_deviceUID)
		CFRelease(m_deviceUID);
	if (m_block)
		Block_release(m_block);
	if (m_dispatchQueue)
		dispatch_release(m_dispatchQueue);
}

void AudioQueueInput::callbackThread()
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
		job = nullptr;
		if (m_selfDelete)
		{
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

void AudioQueueInput::post(std::function<void()> job)
{
	{
		std::lock_guard<std::mutex> lock(m_cbMutex);
		m_cbJobs.push_back(std::move(job));
	}
	m_cbCond.notify_all();
}

bool AudioQueueInput::onCallbackQueue() const
{
	return std::this_thread::get_id() == m_cbThread.get_id();
}

void AudioQueueInput::drainCallbacks()
{
	// Every callback already queued on the callback thread runs before this returns.
	if (onCallbackQueue())
		return;
	std::unique_lock<std::mutex> lock(m_cbMutex);
	m_cbCond.wait(lock, [&] { return m_cbJobs.empty() && !m_cbBusy; });
}

void AudioQueueInput::runOnClient(std::function<void()> job)
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
	else if (m_dispatchQueue)
	{
		std::shared_ptr<CallbackGate> gate = m_gate;
		dispatch_async(m_dispatchQueue, ^{
			std::lock_guard<std::recursive_mutex> lock(gate->mutex);
			if (gate->open)
				job();
		});
	}
	else
		post(std::move(job));
}

OSStatus AudioQueueInput::create(const AudioStreamBasicDescription *inFormat, AudioQueueInputCallback inCallbackProc,
		void *inUserData, CFRunLoopRef inCallbackRunLoop, CFStringRef inCallbackRunLoopMode,
		dispatch_queue_t inCallbackQueue, AudioQueueInputCallbackBlock inCallbackBlock,
		AudioQueueInput** newQueue)
{
	// QUIRK: argument checks the header leaves open, as observed:
	//  - a NULL format or callback, a channel count or sample size of zero, and a negative
	//    sample rate give paramErr (-50); an unknown format ID gives
	//    kAudioFormatUnsupportedDataFormatError;
	//  - a sample rate of zero is accepted and means the input device's rate (the stream
	//    description property then reports that rate); macOS 26.7.3 gives paramErr for it;
	//  - a frame size that does not fit the sample size gives
	//    kAudioFormatUnsupportedDataFormatError; macOS 26.7.3 gives paramErr for it.
	// Where the versions differ the older (macOS 10.14.4) behaviour is followed.
	// Not documented except for the zero sample rate. Evidence: black-box observation on
	// macOS 10.14.4 x86_64 and 26.7.3 x86_64 (tier 2). Test: part1() in
	// tests/src/audioqueue_input.c
	if (!newQueue || !inFormat || (!inCallbackProc && !inCallbackBlock))
		return kAudio_ParamError;
	if (inFormat->mFormatID != kAudioFormatLinearPCM)
		return kAudioFormatUnsupportedDataFormatError; // no encoders: compressed input is not supported
	if (!(inFormat->mSampleRate >= 0) || inFormat->mChannelsPerFrame == 0 || inFormat->mBitsPerChannel == 0)
		return kAudio_ParamError;

	const bool nonInterleaved = inFormat->mFormatFlags & kAudioFormatFlagIsNonInterleaved;
	const UInt32 channelsInFrame = nonInterleaved ? 1 : inFormat->mChannelsPerFrame;
	const UInt32 minSampleBytes = (inFormat->mBitsPerChannel + 7) / 8;
	if (inFormat->mBytesPerFrame == 0 || inFormat->mBytesPerFrame % channelsInFrame
			|| inFormat->mBytesPerFrame / channelsInFrame < minSampleBytes
			|| ((inFormat->mFormatFlags & kAudioFormatFlagIsPacked) && inFormat->mBytesPerFrame != channelsInFrame * minSampleBytes))
		return kAudioFormatUnsupportedDataFormatError;

	// Only interleaved data (a single channel is the same either way): a buffer cannot carry
	// planes. Packet size must agree with the frame size for linear PCM.
	if (nonInterleaved && inFormat->mChannelsPerFrame > 1)
		return kAudioFormatUnsupportedDataFormatError;
	if (inFormat->mFramesPerPacket != 1 || inFormat->mBytesPerPacket != inFormat->mBytesPerFrame)
		return kAudioFormatUnsupportedDataFormatError;

	AudioQueueInput* queue = new AudioQueueInput(inFormat, inCallbackProc, inUserData, inCallbackRunLoop,
			inCallbackRunLoopMode, inCallbackQueue, inCallbackBlock);

	if (queue->m_format.mSampleRate == 0)
	{
		// The device's rate, as far as it is known now; it is read again when the device is
		// chosen and when recording starts.
		queue->m_rateFromDevice = true;
		queue->refreshRate(queue->resolveDevice());
	}
	else if (queue->configure() != noErr)
	{
		queue->dispose(true);
		return kAudioFormatUnsupportedDataFormatError;
	}
	*newQueue = queue;
	return noErr;
}

// Decides what the unit delivers (the queue's format when it is a plain native float32 or
// signed 16/32-bit interleaved one, otherwise float32 at the same rate and channel count,
// converted) once the sample rate is known.
OSStatus AudioQueueInput::configure()
{
	const AudioStreamBasicDescription& format = m_format;
	const UInt32 flags = format.mFormatFlags;
	const UInt32 bits = format.mBitsPerChannel;
	const bool nonInterleaved = flags & kAudioFormatFlagIsNonInterleaved;
	const bool native = !(flags & kAudioFormatFlagIsBigEndian) == !(kAudioFormatFlagsNativeEndian & kAudioFormatFlagIsBigEndian);
	const bool plain = native && !nonInterleaved && (flags & kAudioFormatFlagIsPacked)
			&& ((flags & kAudioFormatFlagIsFloat) ? bits == 32
				: ((flags & kAudioFormatFlagIsSignedInteger) && (bits == 16 || bits == 32)));

	if (m_converter)
	{
		AudioConverterDispose(m_converter);
		m_converter = nullptr;
	}
	if (plain)
	{
		m_unitFormat = format;
	}
	else
	{
		AudioStreamBasicDescription f = {};
		f.mSampleRate = format.mSampleRate;
		f.mFormatID = kAudioFormatLinearPCM;
		f.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
		f.mChannelsPerFrame = format.mChannelsPerFrame;
		f.mBitsPerChannel = 32;
		f.mFramesPerPacket = 1;
		f.mBytesPerFrame = f.mBytesPerPacket = 4 * f.mChannelsPerFrame;
		OSStatus status = AudioConverterNew(&f, &format, &m_converter);
		if (status != noErr)
			return status;
		m_unitFormat = f;
	}

	mach_timebase_info_data_t timebase;
	mach_timebase_info(&timebase);
	m_hostTicksPerFrame = timebase.numer ? 1e9 / format.mSampleRate * timebase.denom / timebase.numer : 0;
	return noErr;
}

// ---- device and unit ----

AudioDeviceID AudioQueueInput::resolveDevice()
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
		AudioObjectPropertyAddress address = { kAudioHardwarePropertyDefaultInputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
		UInt32 size = sizeof(device);
		if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &size, &device) != noErr)
			device = kAudioObjectUnknown;
	}
	if (device == kAudioObjectUnknown)
		return device;

	// It must be able to record.
	AudioObjectPropertyAddress streams = { kAudioDevicePropertyStreams, kAudioObjectPropertyScopeInput, kAudioObjectPropertyElementMaster };
	UInt32 size = 0;
	if (AudioObjectGetPropertyDataSize(device, &streams, 0, nullptr, &size) != noErr || size == 0)
		return kAudioObjectUnknown;
	return device;
}

Float64 AudioQueueInput::deviceSampleRate(AudioDeviceID device)
{
	if (device == kAudioObjectUnknown)
		return 0;
	Float64 rate = 0;
	UInt32 size = sizeof(rate);
	AudioObjectPropertyAddress address = { kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeInput, kAudioObjectPropertyElementMaster };
	if (AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &rate) != noErr)
		return 0;
	return rate;
}

// For a queue created with a sample rate of zero: take the rate of the device.
bool AudioQueueInput::refreshRate(AudioDeviceID device)
{
	Float64 rate = deviceSampleRate(device);
	if (rate <= 0)
		return false;
	if (rate == m_format.mSampleRate)
		return true;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_format.mSampleRate = rate;
	}
	return configure() == noErr;
}

OSStatus AudioQueueInput::ensureUnit()
{
	if (m_unit)
		return noErr;
	AudioDeviceID device = resolveDevice();
	if (device == kAudioObjectUnknown)
		return kAudioQueueErr_InvalidDevice;
	if (m_rateFromDevice && !refreshRate(device))
		return kAudioQueueErr_InvalidDevice;

	AudioComponentDescription desc = { kAudioUnitType_Output, kAudioUnitSubType_HALOutput, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponent component = AudioComponentFindNext(nullptr, &desc);
	if (!component)
		return kAudioQueueErr_InvalidDevice;
	AudioComponentInstance unit;
	OSStatus status = AudioComponentInstanceNew(component, &unit);
	if (status != noErr)
		return status;

	UInt32 on = 1, off = 0;
	AURenderCallbackStruct callback = { inputCallback, this };
	status = AudioUnitSetProperty(unit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, kInputBus, &on, sizeof(on));
	if (status == noErr)
		status = AudioUnitSetProperty(unit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, kOutputBus, &off, sizeof(off));
	if (status == noErr)
		status = AudioUnitSetProperty(unit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &device, sizeof(device));
	if (status == noErr)
		status = AudioUnitSetProperty(unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, kInputBus, &m_unitFormat, sizeof(m_unitFormat));
	if (status == noErr)
		status = AudioUnitSetProperty(unit, kAudioOutputUnitProperty_SetInputCallback, kAudioUnitScope_Global, 0, &callback, sizeof(callback));
	UInt32 maxFrames = 0;
	UInt32 size = sizeof(maxFrames);
	if (status == noErr)
		status = AudioUnitGetProperty(unit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &maxFrames, &size);
	if (status == noErr && maxFrames == 0)
		status = kAudioQueueErr_InvalidDevice;
	if (status == noErr)
		status = AudioUnitInitialize(unit);
	if (status != noErr)
	{
		AudioComponentInstanceDispose(unit);
		return status == kAudioQueueErr_InvalidDevice ? status : kAudioQueueErr_CannotStart;
	}

	m_maxSliceFrames = maxFrames;
	m_scratch.assign(size_t(maxFrames) * m_unitFormat.mBytesPerFrame, 0);
	if (m_converter)
		m_converted.assign(size_t(maxFrames) * m_format.mBytesPerFrame, 0);
	m_unit = unit;
	return noErr;
}

void AudioQueueInput::destroyUnit()
{
	if (!m_unit)
		return;
	AudioOutputUnitStop(m_unit);
	AudioUnitUninitialize(m_unit);
	AudioComponentInstanceDispose(m_unit);
	m_unit = nullptr;
}

// ---- capture ----

OSStatus AudioQueueInput::inputCallback(void* refCon, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* timeStamp,
	UInt32 busNumber, UInt32 numberFrames, AudioBufferList* ioData)
{
	return static_cast<AudioQueueInput*>(refCon)->capture(flags, timeStamp, numberFrames);
}

OSStatus AudioQueueInput::capture(AudioUnitRenderActionFlags* flags, const AudioTimeStamp* timeStamp, UInt32 frames)
{
	if (!frames || frames > m_maxSliceFrames)
		return noErr;

	AudioBufferList list = {};
	list.mNumberBuffers = 1;
	list.mBuffers[0].mNumberChannels = m_unitFormat.mChannelsPerFrame;
	list.mBuffers[0].mDataByteSize = frames * m_unitFormat.mBytesPerFrame;
	list.mBuffers[0].mData = m_scratch.data();
	OSStatus status = AudioUnitRender(m_unit, flags, timeStamp, kInputBus, frames, &list);
	if (status != noErr)
		return status;

	updateLevels(m_scratch.data(), frames);

	const uint8_t* data = m_scratch.data();
	if (m_converter)
	{
		AudioBufferList out = {};
		out.mNumberBuffers = 1;
		out.mBuffers[0].mNumberChannels = m_format.mChannelsPerFrame;
		out.mBuffers[0].mDataByteSize = frames * m_format.mBytesPerFrame;
		out.mBuffers[0].mData = m_converted.data();
		status = AudioConverterConvertComplexBuffer(m_converter, frames, &list, &out);
		if (status != noErr)
		{
			m_converterError.store(UInt32(status), std::memory_order_relaxed);
			return noErr;
		}
		data = m_converted.data();
	}
	append(data, frames, timeStamp);
	return noErr;
}

// Peak and RMS of the slice, per channel, kept for kAudioQueueProperty_CurrentLevelMeter.
void AudioQueueInput::updateLevels(const uint8_t* data, UInt32 frames)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_metering)
			return;
	}
	const UInt32 channels = m_unitFormat.mChannelsPerFrame;
	const bool isFloat = m_unitFormat.mFormatFlags & kAudioFormatFlagIsFloat;
	const UInt32 bits = m_unitFormat.mBitsPerChannel;
	std::vector<AudioQueueLevelMeterState> levels(channels, AudioQueueLevelMeterState { 0, 0 });
	std::vector<double> sums(channels, 0.0);
	for (UInt32 i = 0; i < frames; i++)
	{
		for (UInt32 c = 0; c < channels; c++)
		{
			const size_t index = size_t(i) * channels + c;
			double v;
			if (isFloat)
			{
				float f;
				memcpy(&f, data + index * 4, 4);
				v = f;
			}
			else if (bits == 16)
			{
				int16_t s;
				memcpy(&s, data + index * 2, 2);
				v = s / 32768.0;
			}
			else
			{
				int32_t s;
				memcpy(&s, data + index * 4, 4);
				v = s / 2147483648.0;
			}
			const double a = std::fabs(v);
			if (a > levels[c].mPeakPower)
				levels[c].mPeakPower = float(a);
			sums[c] += v * v;
		}
	}
	for (UInt32 c = 0; c < channels; c++)
		levels[c].mAveragePower = float(std::sqrt(sums[c] / frames));

	std::lock_guard<std::mutex> lock(m_mutex);
	m_levels = std::move(levels);
}

void AudioQueueInput::append(const uint8_t* data, UInt32 frames, const AudioTimeStamp* timeStamp)
{
	std::vector<Filled> filled;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_running || m_paused || m_stopPending)
			return;

		const UInt32 frameBytes = m_format.mBytesPerFrame;
		UInt32 done = 0;
		while (done < frames)
		{
			if (m_pending.empty())
			{
				// Nothing to record into: these frames are lost, but time goes on.
				m_frames += frames - done;
				break;
			}
			AudioQueueBufferRef buffer = m_pending.front();
			const UInt32 capacity = buffer->mAudioDataBytesCapacity / frameBytes * frameBytes;
			if (m_curBytes == 0)
			{
				memset(&m_curStart, 0, sizeof(m_curStart));
				m_curStart.mSampleTime = Float64(m_frames);
				m_curStart.mFlags = kAudioTimeStampSampleTimeValid;
				if (timeStamp && (timeStamp->mFlags & kAudioTimeStampHostTimeValid))
				{
					m_curStart.mHostTime = timeStamp->mHostTime + UInt64(done * m_hostTicksPerFrame);
					m_curStart.mFlags |= kAudioTimeStampHostTimeValid;
				}
			}
			const UInt32 take = std::min<UInt32>(capacity - m_curBytes, (frames - done) * frameBytes);
			memcpy(static_cast<uint8_t*>(buffer->mAudioData) + m_curBytes, data + size_t(done) * frameBytes, take);
			m_curBytes += take;
			done += take / frameBytes;
			m_frames += take / frameBytes;
			if (m_curBytes >= capacity)
			{
				buffer->mAudioDataByteSize = m_curBytes;
				buffer->mPacketDescriptionCount = 0;
				filled.push_back({ buffer, m_curStart });
				m_pending.pop_front();
				m_curBytes = 0;
			}
		}
	}
	if (!filled.empty())
		returnBuffers(filled);
}

// ---- returning buffers to the client ----

void AudioQueueInput::takePending(std::vector<Filled>& out)
{
	AudioTimeStamp now = {};
	now.mSampleTime = Float64(m_frames);
	now.mFlags = kAudioTimeStampSampleTimeValid;
	bool first = true;
	for (AudioQueueBufferRef buffer : m_pending)
	{
		if (first && m_curBytes)
		{
			// The buffer being filled goes back with what it holds.
			buffer->mAudioDataByteSize = m_curBytes;
			out.push_back({ buffer, m_curStart });
		}
		else
		{
			buffer->mAudioDataByteSize = 0;
			out.push_back({ buffer, now });
		}
		buffer->mPacketDescriptionCount = 0;
		first = false;
	}
	m_pending.clear();
	m_curBytes = 0;
}

void AudioQueueInput::returnBuffers(const std::vector<Filled>& buffers)
{
	for (const Filled& filled : buffers)
	{
		AudioQueueInputCallback callback = m_callback;
		AudioQueueInputCallbackBlock block = m_block;
		void* userData = m_userData;
		AudioQueueRef queue = this;
		Filled f = filled;
		runOnClient([=] {
			// QUIRK: for linear PCM the callback gets zero packet descriptions and a NULL
			// description array, not the number of frames. Not stated by the header; the
			// header says the count is of the packets in the data. Evidence: Apple's published
			// recording sample code treats a zero count for constant-bit-rate formats as "work the
			// packet count out from the byte size" (tier 1, no macOS recording device was
			// available for black-box observation). Test: test_basic_recording()
			// in tests/src/audioqueue_input.c
			if (block)
				block(queue, f.buffer, &f.start, 0, nullptr);
			else
				callback(userData, queue, f.buffer, &f.start, 0, nullptr);
		});
	}
}

// ---- control ----

OSStatus AudioQueueInput::start(const AudioTimeStamp *inStartTime)
{
	// A start time on the device's timeline is not supported: recording begins at once.
	std::lock_guard<std::mutex> control(m_controlMutex);
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_running && !m_stopPending)
		{
			m_paused = false;
			return noErr;
		}
	}
	OSStatus status = ensureUnit();
	if (status != noErr)
		return status;

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_running)
		{
			// Restarted while a deferred stop was draining: the unit is still running.
			m_stopPending = false;
			m_paused = false;
			return noErr;
		}
		m_frames = 0;
		m_curBytes = 0;
		m_paused = false;
		m_stopPending = false;
		m_running = true;
	}
	status = AudioOutputUnitStart(m_unit);
	if (status != noErr)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_running = false;
		return kAudioQueueErr_CannotStart;
	}
	notifyListeners(kAudioQueueProperty_IsRunning);
	return noErr;
}

OSStatus AudioQueueInput::prime(UInt32 inNumberOfFramesToPrepare, UInt32 *outNumberOfFramesPrepared)
{
	// Priming is for playback.
	return kAudioQueueErr_InvalidQueueType;
}

OSStatus AudioQueueInput::flush()
{
	// Captured frames go into the buffers as they arrive; nothing is held back.
	return noErr;
}

void AudioQueueInput::finishStop()
{
	std::lock_guard<std::mutex> control(m_controlMutex);
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_stopPending)
			return; // started again meanwhile
		m_running = false;
		m_stopPending = false;
		m_paused = false;
	}
	if (m_unit)
		AudioOutputUnitStop(m_unit);
	notifyListeners(kAudioQueueProperty_IsRunning);
}

OSStatus AudioQueueInput::stop(Boolean inImmediate)
{
	if (!inImmediate)
	{
		// Asynchronous: recording ends now, the buffers (the one being filled with what it
		// holds, the others empty) go back to the client, and the queue then reports it has
		// stopped, in that order on the callback context.
		std::vector<Filled> returned;
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_running || m_stopPending)
				return noErr;
			m_stopPending = true;
			takePending(returned);
		}
		returnBuffers(returned);
		runOnClient([this] { finishStop(); });
		return noErr;
	}

	std::unique_lock<std::mutex> control(m_controlMutex);
	bool wasRunning;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		wasRunning = m_running;
		m_running = false;
		m_stopPending = false;
		m_paused = false;
	}
	if (!wasRunning)
		return noErr;
	// Returns when the capture thread has unwound: nothing is recorded after it.
	if (m_unit)
		AudioOutputUnitStop(m_unit);
	std::vector<Filled> returned;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		takePending(returned);
		m_frames = 0;
	}
	control.unlock();
	returnBuffers(returned);
	notifyListeners(kAudioQueueProperty_IsRunning);
	drainCallbacks();
	return noErr;
}

OSStatus AudioQueueInput::pause()
{
	// Frames captured while paused are dropped and do not advance the queue's time.
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_running)
		m_paused = true;
	return noErr;
}

OSStatus AudioQueueInput::reset()
{
	std::vector<Filled> returned;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		takePending(returned);
	}
	returnBuffers(returned);
	drainCallbacks();
	return noErr;
}

OSStatus AudioQueueInput::dispose(Boolean inImmediate)
{
	if (onCallbackQueue())
	{
		// Called from a callback on the queue's thread: finish on that thread, after it.
		std::lock_guard<std::mutex> control(m_controlMutex);
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_running = false;
			m_pending.clear();
			m_curBytes = 0;
		}
		if (m_unit)
			AudioOutputUnitStop(m_unit);
		post([this] { m_selfDelete = true; });
		return noErr;
	}
	// Disposing stops first and returns no buffers: they are freed with the queue.
	{
		std::lock_guard<std::mutex> control(m_controlMutex);
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_running = false;
			m_stopPending = false;
		}
		if (m_unit)
			AudioOutputUnitStop(m_unit);
		std::lock_guard<std::mutex> lock(m_mutex);
		m_pending.clear();
	}
	delete this;
	return noErr;
}

// ---- buffers ----

OSStatus AudioQueueInput::allocateBuffer(UInt32 inBufferByteSize, UInt32 inNumberPacketDescriptions, AudioQueueBufferRef *outBuffer)
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

OSStatus AudioQueueInput::freeBuffer(AudioQueueBufferRef inBuffer)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_allocated.count(inBuffer))
		return kAudioQueueErr_InvalidBuffer;
	if (std::find(m_pending.begin(), m_pending.end(), inBuffer) != m_pending.end())
		return kAudioQueueErr_BufferInQueue;
	m_allocated.erase(inBuffer);
	free(inBuffer->mAudioData);
	free(inBuffer->mPacketDescriptions);
	free(inBuffer);
	return noErr;
}

OSStatus AudioQueueInput::enqueueBuffer(AudioQueueBufferRef inBuffer, UInt32 inNumPacketDescs, const AudioStreamPacketDescription *inPacketDescs)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_allocated.count(inBuffer))
		return kAudioQueueErr_InvalidBuffer;
	if (std::find(m_pending.begin(), m_pending.end(), inBuffer) != m_pending.end())
		return kAudioQueueErr_BufferEnqueuedTwice;
	// The byte size and the packet descriptions a client leaves in a buffer it enqueues for
	// recording mean nothing: the size is rewritten when the buffer is filled. (macOS 26.7.3
	// refuses a size above the capacity with paramErr, macOS 10.14.4 accepts it; the older
	// behaviour is followed.)
	inBuffer->mAudioDataByteSize = 0;
	m_pending.push_back(inBuffer);
	return noErr;
}

OSStatus AudioQueueInput::getCurrentTime(AudioTimeStamp *outTimeStamp, Boolean *outTimelineDiscontinuity)
{
	if (!outTimeStamp)
		return kAudioQueueErr_InvalidParameter;
	UInt64 frames;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_running)
			return kAudioQueueErr_InvalidRunState;
		frames = m_frames;
	}
	memset(outTimeStamp, 0, sizeof(*outTimeStamp));
	outTimeStamp->mSampleTime = Float64(frames);
	outTimeStamp->mHostTime = mach_absolute_time();
	outTimeStamp->mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid;
	if (outTimelineDiscontinuity)
		*outTimelineDiscontinuity = false;
	return noErr;
}

// ---- properties ----

OSStatus AudioQueueInput::getPropertySize(AudioQueuePropertyID inID, UInt32 *outDataSize)
{
	if (!outDataSize)
		return kAudioQueueErr_InvalidParameter;
	switch (inID)
	{
		case kAudioQueueProperty_IsRunning:
		case kAudioQueueDeviceProperty_NumberChannels:
		case kAudioQueueProperty_EnableLevelMetering:
		case kAudioQueueProperty_ConverterError:
		case kAudioQueueProperty_MaximumOutputPacketSize:
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
	}
	return kAudioQueueErr_InvalidProperty;
}

OSStatus AudioQueueInput::getProperty(AudioQueuePropertyID inID, void *outData, UInt32 *ioDataSize)
{
	if (!outData || !ioDataSize)
		return kAudioQueueErr_InvalidParameter;
	UInt32 need;
	OSStatus status = getPropertySize(inID, &need);
	if (status != noErr)
		return status;

	// QUIRK: the running flag must be read with a buffer of exactly four bytes: a larger one,
	// like a smaller one, gives kAudioQueueErr_InvalidPropertySize. Not documented.
	// Evidence: black-box observation on macOS 26.7.3 x86_64 (tier 2).
	// Test: part1() in tests/src/audioqueue_input.c
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
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			memcpy(outData, &m_format, sizeof(m_format));
			break;
		}
		case kAudioQueueProperty_ConverterError:
			*static_cast<UInt32*>(outData) = m_converterError.load(std::memory_order_relaxed);
			break;
		case kAudioQueueProperty_MaximumOutputPacketSize:
			*static_cast<UInt32*>(outData) = m_format.mBytesPerPacket;
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
			for (UInt32 c = 0; c < m_levels.size(); c++)
			{
				out[c] = m_levels[c];
				if (inID == kAudioQueueProperty_CurrentLevelMeterDB)
				{
					// Decibels relative to full scale; silence is reported as -160 dB.
					out[c].mAveragePower = 20 * std::log10(std::max(out[c].mAveragePower, 1e-8f));
					out[c].mPeakPower = 20 * std::log10(std::max(out[c].mPeakPower, 1e-8f));
				}
			}
			break;
		}
		case kAudioQueueProperty_CurrentDevice:
		case kAudioQueueDeviceProperty_SampleRate:
		case kAudioQueueDeviceProperty_NumberChannels:
		{
			AudioDeviceID device = resolveDevice();
			if (device == kAudioObjectUnknown)
				return kAudioQueueErr_InvalidDevice;
			if (inID == kAudioQueueProperty_CurrentDevice)
			{
				CFStringRef uid = nullptr;
				UInt32 size = sizeof(uid);
				AudioObjectPropertyAddress address = { kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
				if (AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &uid) != noErr || !uid)
					return kAudioQueueErr_InvalidDevice;
				*static_cast<CFStringRef*>(outData) = uid; // the caller releases it
			}
			else if (inID == kAudioQueueDeviceProperty_SampleRate)
			{
				Float64 rate = deviceSampleRate(device);
				if (rate <= 0)
					return kAudioQueueErr_InvalidDevice;
				*static_cast<Float64*>(outData) = rate;
			}
			else
			{
				AudioObjectPropertyAddress address = { kAudioDevicePropertyStreamConfiguration, kAudioObjectPropertyScopeInput, kAudioObjectPropertyElementMaster };
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

OSStatus AudioQueueInput::setProperty(AudioQueuePropertyID inID, const void *inData, UInt32 inDataSize)
{
	if (!inData)
		return kAudioQueueErr_InvalidParameter;
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
			if (!uid || CFGetTypeID(uid) != CFStringGetTypeID())
				return kAudioQueueErr_InvalidPropertyValue;
			// The device is changed while the queue is stopped. An unknown UID is
			// reported when the queue is started.
			std::lock_guard<std::mutex> control(m_controlMutex);
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				if (m_running)
					return kAudioQueueErr_InvalidRunState;
				if (m_deviceUID)
					CFRelease(m_deviceUID);
				m_deviceUID = static_cast<CFStringRef>(CFRetain(uid));
			}
			destroyUnit();
			if (m_rateFromDevice)
				refreshRate(resolveDevice());
			return noErr;
		}
		case kAudioQueueProperty_IsRunning:
		case kAudioQueueProperty_StreamDescription:
		case kAudioQueueProperty_CurrentLevelMeter:
		case kAudioQueueProperty_CurrentLevelMeterDB:
		case kAudioQueueProperty_ConverterError:
		case kAudioQueueProperty_MaximumOutputPacketSize:
		case kAudioQueueDeviceProperty_SampleRate:
		case kAudioQueueDeviceProperty_NumberChannels:
			return kAudioQueueErr_InvalidProperty; // read-only
	}
	return kAudioQueueErr_InvalidProperty;
}

OSStatus AudioQueueInput::setOfflineRenderFormat(const AudioStreamBasicDescription *inFormat, const AudioChannelLayout *inLayout)
{
	return kAudioQueueErr_InvalidQueueType;
}

OSStatus AudioQueueInput::offlineRender(const AudioTimeStamp *inTimestamp, AudioQueueBufferRef ioBuffer, UInt32 inNumberFrames)
{
	return kAudioQueueErr_InvalidQueueType;
}
