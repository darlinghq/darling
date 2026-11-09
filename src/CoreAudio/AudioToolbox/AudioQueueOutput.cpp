#include "AudioQueueOutput.h"
#include "stub.h"
#include <CarbonCore/MacErrors.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioFormat.h>
#include <CoreAudio/AudioHardware.h>
#include <mach/mach_time.h>
#include <cmath>
#include <cstdlib>
#include <cstring>

static const UInt32 kDeviceBufferFrames = 2048;

AudioQueueOutput::AudioQueueOutput(const AudioStreamBasicDescription *inFormat,
		AudioQueueOutputCallback inCallbackProc,
	void *inUserData, CFRunLoopRef inCallbackRunLoop,
	CFStringRef inCallbackRunLoopMode, UInt32 inFlags)
: AudioQueue(inFormat, inUserData, inCallbackRunLoop, inCallbackRunLoopMode, inFlags),
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

OSStatus AudioQueueOutput::render(UInt32 numberFrames, AudioBufferList* ioData)
{
	std::vector<AudioQueueBufferRef> finished;
	const UInt32 frameBytes = m_format.mBytesPerFrame;
	bool stopNow = false;
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		for (UInt32 b = 0; b < ioData->mNumberBuffers; b++)
		{
			uint8_t* out = static_cast<uint8_t*>(ioData->mBuffers[b].mData);
			UInt32 want = std::min<UInt32>(ioData->mBuffers[b].mDataByteSize, numberFrames * frameBytes);
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
			AudioOutputUnitStop(m_unit);
		});
	}
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
	OSStatus status = ensureUnit();
	if (status != noErr)
		return status;

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_paused = false;
		m_stopWhenDrained = false;
		if (m_running)
			return noErr;
	}
	status = AudioOutputUnitStart(m_unit);
	if (status != noErr)
		return status;
	std::lock_guard<std::mutex> lock(m_mutex);
	m_running = true;
	return noErr;
}

OSStatus AudioQueueOutput::prime(UInt32 inNumberOfFramesToPrepare, UInt32 *outNumberOfFramesPrepared)
{
	// Nothing is decoded ahead of time: linear PCM is played as it is.
	std::unique_lock<std::mutex> control(m_controlMutex);
	OSStatus status = ensureUnit();
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
	STUB();
	return unimpErr;
}

OSStatus AudioQueueOutput::offlineRender(const AudioTimeStamp *inTimestamp, AudioQueueBufferRef ioBuffer, UInt32 inNumberFrames)
{
	STUB();
	return unimpErr;
}
