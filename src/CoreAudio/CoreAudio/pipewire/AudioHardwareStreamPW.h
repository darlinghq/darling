/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef AUDIOHARDWARESTREAMPW_H
#define AUDIOHARDWARESTREAMPW_H

#include "../AudioHardwareStream.h"
#include <CoreAudio/AudioHardware.h>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

struct darling_pw_buffer;
struct darling_pw_stream;
class AudioHardwareImplPW;

// One HAL IOProc attached to one PipeWire stream. The shared PipeWire client
// gives us buffer dequeue/queue operations and a process event, so each stream
// owns a pump thread that drains the buffers PipeWire makes available.
class AudioHardwareStreamPW : public AudioHardwareStream
{
public:
	AudioHardwareStreamPW(AudioHardwareImplPW* hw, AudioDeviceIOProc callback,
		void* clientData, bool capture, const char* role);
	~AudioHardwareStreamPW() override;

	void stop() override;
	bool valid() const { return m_stream != nullptr; }

protected:
	// Called with the PipeWire stream lock held; the derived class renders into
	// or reads from the buffer and returns it to PipeWire.
	virtual void processBuffer(const darling_pw_buffer& buffer, void* cookie) = 0;

	struct Timestamps
	{
		AudioTimeStamp now;
		AudioTimeStamp io;
	};

	Timestamps timestamps(uint64_t frameCount, bool capture);
	uint32_t blockFrames() const;
	uint32_t frameBytes() const { return m_frameBytes; }
	uint32_t channels() const { return m_channels; }
	bool running() const { return m_running.load(std::memory_order_relaxed); }
	uint64_t frameCount() const { return m_frames.load(std::memory_order_relaxed); }
	// Returns the frame number that the block starts at.
	uint64_t advanceFrames(uint32_t frames)
	{
		return m_frames.fetch_add(frames, std::memory_order_relaxed);
	}

protected:
	void pump();
	static void onStateChanged(void* userData, int state);
	void wake();

	AudioHardwareImplPW* m_hw;
	AudioDeviceIOProc m_callback;
	void* m_clientData;
	bool m_capture;
	uint32_t m_frameBytes;
	uint32_t m_channels;
	double m_rate;

	darling_pw_stream* m_stream = nullptr;
	std::atomic<bool> m_running{false};
	std::atomic<uint64_t> m_frames{0};
	uint64_t m_lastInputHost = 0;

	std::thread m_thread;
	std::mutex m_wakeMutex;
	std::condition_variable m_wake;
	// Set by the process event, cleared by the pump.
	unsigned m_pending = 0;
};

#endif /* AUDIOHARDWARESTREAMPW_H */
