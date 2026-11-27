/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "AudioHardwareStreamPW.h"
#include "AudioHardwareImplPW.h"
#include "PWConfig.h"
#include <CoreAudio/HostTime.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>

namespace {
// A pump iteration serves a few buffers before yielding, so no single IOProc
// can starve the other streams of a device.
const int kMaxBuffersPerWake = 8;
const int kIdleWaitMilliseconds = 2;
const uint32_t kMaxBlockFrames = 4096;

void fillTimeStamp(AudioTimeStamp& stamp, uint64_t hostTime, double sampleTime)
{
	std::memset(&stamp, 0, sizeof(stamp));
	stamp.mHostTime = hostTime;
	stamp.mSampleTime = sampleTime;
	stamp.mRateScalar = 1.0;
	stamp.mFlags = AudioTimeStampFlags(kAudioTimeStampHostTimeValid
		| kAudioTimeStampSampleTimeValid | kAudioTimeStampRateScalarValid);
}
}

AudioHardwareStreamPW::AudioHardwareStreamPW(AudioHardwareImplPW* hw,
	AudioDeviceIOProc callback, void* clientData, bool capture, const char* role)
: AudioHardwareStream(hw, false)
, m_hw(hw)
, m_callback(callback)
, m_clientData(clientData)
, m_capture(capture)
, m_frameBytes(hw->asbd().mBytesPerFrame ? hw->asbd().mBytesPerFrame
		: hw->asbd().mChannelsPerFrame * sizeof(float))
, m_channels(hw->asbd().mChannelsPerFrame ? hw->asbd().mChannelsPerFrame : 2)
, m_rate(hw->asbd().mSampleRate > 0 ? hw->asbd().mSampleRate : 44100.0)
{
	darling_pw_stream_config config;
	if (!PWStreamConfigForASBD(hw->asbd(), capture, role, &config))
		return;

	darling_pw_stream_events events{};
	events.version = 1;
	events.state_changed = &AudioHardwareStreamPW::onStateChanged;
	events.process = [](void* userData) {
		static_cast<AudioHardwareStreamPW*>(userData)->wake();
	};

	if (darling_pw_stream_create(&config, -1, &events, this, &m_stream) != 0)
	{
		m_stream = nullptr;
		return;
	}

	// A stream may still be negotiating its format. Waiting here keeps the HAL
	// from reporting a successful AudioDeviceStart() for a stream that cannot
	// produce or consume data yet.
	if (darling_pw_stream_wait(m_stream, 500) < 0)
	{
		darling_pw_stream_destroy(m_stream);
		m_stream = nullptr;
		return;
	}
	m_running.store(true, std::memory_order_relaxed);
	m_thread = std::thread([this] { pump(); });
}

AudioHardwareStreamPW::~AudioHardwareStreamPW()
{
	stop();
	if (m_stream)
	{
		darling_pw_stream_destroy(m_stream);
		m_stream = nullptr;
	}
}

void AudioHardwareStreamPW::stop()
{
	if (m_running.exchange(false, std::memory_order_relaxed))
		wake();
	if (m_stream)
		darling_pw_stream_set_active(m_stream, 0);
	// An IOProc may stop its own stream. Joining the current thread would
	// deadlock; the device defers deletion of such a stream to another queue.
	if (m_thread.joinable() && m_thread.get_id() != std::this_thread::get_id())
		m_thread.join();
}

void AudioHardwareStreamPW::wake()
{
	{
		std::lock_guard<std::mutex> lock(m_wakeMutex);
		++m_pending;
	}
	m_wake.notify_all();
}

void AudioHardwareStreamPW::onStateChanged(void* userData, int)
{
	static_cast<AudioHardwareStreamPW*>(userData)->wake();
}

uint32_t AudioHardwareStreamPW::blockFrames() const
{
	if (!m_frameBytes)
		return 1;
	const uint32_t frames = m_hw->bufferSize() / m_frameBytes;
	return std::min(std::max(frames, uint32_t(1)), kMaxBlockFrames);
}

AudioHardwareStreamPW::Timestamps AudioHardwareStreamPW::timestamps(uint64_t frameCount,
	bool capture)
{
	Timestamps result;
	darling_pw_time time{};
	long double delayNs = 0;
	if (darling_pw_stream_get_time(m_stream, &time) == 0 && time.rate_num > 0
		&& time.rate_denom > 0 && time.delay_ticks > 0)
	{
		// The ticks are in units of rate.num / rate.denom seconds (1/sample rate).
		delayNs = static_cast<long double>(time.delay_ticks) * 1000000000.0L
			* time.rate_num / time.rate_denom;
	}
	delayNs = std::min(std::max(delayNs, 0.0L), 60000000000.0L);

	const uint64_t nowHost = AudioGetCurrentHostTime();
	const uint64_t delayHost = AudioConvertNanosToHostTime(static_cast<UInt64>(delayNs));
	const double delayFrames = m_rate > 0 ? double(delayNs) * m_rate / 1e9 : 0;
	if (!capture)
		m_hw->setLatencyFrames(delayFrames < 0xffffffff ? uint32_t(delayFrames) : 0xffffffffu);

	if (!capture)
	{
		const double playing = std::max(0.0, double(frameCount) - delayFrames);
		fillTimeStamp(result.now, nowHost, playing);
		fillTimeStamp(result.io, nowHost + delayHost, double(frameCount));
		return result;
	}

	// The oldest unread capture was taken `delayNs` ago; the current block's
	// host time follows it and is kept monotonic across buffers.
	uint64_t inputHost = nowHost - std::min(nowHost, delayHost);
	if (inputHost <= m_lastInputHost)
		inputHost = m_lastInputHost + 1;
	m_lastInputHost = inputHost;

	const uint64_t nowHostForCallback = std::max(nowHost, inputHost);
	const double elapsedFrames = m_rate > 0
		? double(AudioConvertHostTimeToNanos(nowHostForCallback - inputHost)) * m_rate / 1e9
		: 0;
	fillTimeStamp(result.io, inputHost, double(frameCount));
	fillTimeStamp(result.now, nowHostForCallback, double(frameCount) + elapsedFrames);
	return result;
}

void AudioHardwareStreamPW::pump()
{
	while (m_running.load(std::memory_order_relaxed))
	{
		{
			// Serve whatever PipeWire has ready each time the process event fires,
			// and at least every few milliseconds in case the event was reported
			// while the previous batch was being served. The event is counted under
			// the mutex so that a wake-up cannot fall between the check and the wait.
			std::unique_lock<std::mutex> lock(m_wakeMutex);
			m_wake.wait_for(lock, std::chrono::milliseconds(kIdleWaitMilliseconds),
				[this] { return m_pending > 0 || !running(); });
			m_pending = 0;
		}

		if (darling_pw_stream_lock(m_stream) == 0)
		{
			if (darling_pw_stream_state(m_stream) == DARLING_PW_STATE_STREAMING)
			{
				for (int i = 0; i < kMaxBuffersPerWake && running(); ++i)
				{
					darling_pw_buffer buffer{};
					void* cookie = nullptr;
					if (darling_pw_stream_dequeue(m_stream, &buffer, &cookie) <= 0)
						break;
					processBuffer(buffer, cookie);
				}
			}
			darling_pw_stream_unlock(m_stream);
		}
	}
}
