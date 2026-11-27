/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "AudioHardwareStreamPWInput.h"
#include "AudioHardwareImplPW.h"
#include "PWConfig.h"
#include <algorithm>

void AudioHardwareStreamPWInput::processBuffer(const darling_pw_buffer& buffer,
	void* cookie)
{
	const uint32_t bytesPerFrame = frameBytes();
	size_t used = 0;
	if (buffer.data && bytesPerFrame)
	{
		const size_t available = std::min<size_t>(buffer.filled, buffer.capacity);
		const uint64_t startFrame = frameCount();

		// A PipeWire capture buffer can hold more than one HAL IOProc block.
		// Each callback sees one block, as it would on a real audio device.
		while (running() && used + bytesPerFrame <= available)
		{
			const uint32_t frames = std::min<uint32_t>(blockFrames(),
				PWFramesForBytes(available - used, bytesPerFrame));
			if (!frames)
				break;

			const size_t bytes = size_t(frames) * bytesPerFrame;
			const Timestamps stamp = timestamps(startFrame + PWFramesForBytes(used, bytesPerFrame), true);

			AudioBufferList list{};
			list.mNumberBuffers = 1;
			list.mBuffers[0].mNumberChannels = channels();
			list.mBuffers[0].mDataByteSize = static_cast<UInt32>(bytes);
			list.mBuffers[0].mData = static_cast<uint8_t*>(buffer.data) + used;

			// A failing IOProc must not stop delivery of the captured audio.
			m_callback(m_hw->id(), &stamp.now, &list, &stamp.io, nullptr, nullptr,
				m_clientData);

			used += bytes;
			advanceFrames(frames);
			m_hw->advanceSampleClock(frames);
		}
	}

	darling_pw_stream_queue(m_stream, cookie, used);
}
