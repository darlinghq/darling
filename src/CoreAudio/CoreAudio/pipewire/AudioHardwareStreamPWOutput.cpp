/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "AudioHardwareStreamPWOutput.h"
#include "AudioHardwareImplPW.h"
#include "PWConfig.h"
#include <algorithm>
#include <cstring>

void AudioHardwareStreamPWOutput::processBuffer(const darling_pw_buffer& buffer,
	void* cookie)
{
	const uint32_t bytesPerFrame = frameBytes();
	if (!buffer.data || !bytesPerFrame)
	{
		darling_pw_stream_queue(m_stream, cookie, 0);
		return;
	}

	uint32_t frames = blockFrames();
	if (buffer.requested_frames)
		frames = std::min(frames, buffer.requested_frames);
	frames = std::min<uint32_t>(frames, PWFramesForBytes(buffer.capacity, bytesPerFrame));

	const size_t bytes = size_t(frames) * bytesPerFrame;
	// Silence is what gets played when the IOProc does not fill the whole
	// block, fails, or stops the stream from inside its callback.
	std::memset(buffer.data, 0, bytes);

	const uint64_t startFrame = frameCount();
	const Timestamps stamp = timestamps(startFrame, false);

	AudioBufferList list{};
	list.mNumberBuffers = 1;
	list.mBuffers[0].mNumberChannels = channels();
	list.mBuffers[0].mDataByteSize = static_cast<UInt32>(bytes);
	list.mBuffers[0].mData = buffer.data;

	const OSStatus status = m_callback(m_hw->id(), &stamp.now, nullptr, nullptr,
		&list, &stamp.io, m_clientData);

	size_t used = bytes;
	if (running() && status == noErr && list.mBuffers[0].mDataByteSize)
		used = std::min<size_t>(used, list.mBuffers[0].mDataByteSize);
	used -= used % bytesPerFrame;

	// The IOProc is allowed to substitute the buffer it writes into.
	if (used && list.mBuffers[0].mData && list.mBuffers[0].mData != buffer.data)
		std::memcpy(buffer.data, list.mBuffers[0].mData, used);

	darling_pw_stream_queue(m_stream, cookie, used);

	const uint32_t played = PWFramesForBytes(used, bytesPerFrame);
	if (played)
	{
		advanceFrames(played);
		m_hw->advanceSampleClock(played);
	}
}
