/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "AudioHardwareStreamPAOutput.h"
#include "AudioHardwareImplPA.h"
#include <iostream>
#include <algorithm>
#include <mach/mach_time.h>
#include "CoreAudio/HostTime.h"

// Real HALs call an output IOProc with the same frame count on every IO cycle
// (kAudioDevicePropertyBufferFrameSize), so PulseAudio's write requests are served in whole blocks.

static const unsigned kTargetBlocks = 8;

AudioHardwareStreamPAOutput::AudioHardwareStreamPAOutput(AudioHardwareImplPA* hw, AudioDeviceIOProc callback, void* clientData)
: AudioHardwareStreamPA(hw, callback, clientData)
{
	initialize();
}

uint64_t AudioHardwareStreamPAOutput::outputLatencyNs(size_t writable)
{
	pa_usec_t usec = 0;
	int negative = 0;
	if (pa_stream_get_latency(m_stream, &usec, &negative) == 0)
		return negative ? 0 : uint64_t(usec) * 1000;

	// No timing info yet: at least account for what is queued in the PulseAudio buffer.
	const pa_buffer_attr* attr = pa_stream_get_buffer_attr(m_stream);
	const pa_sample_spec* spec = pa_stream_get_sample_spec(m_stream);
	if (!attr || !spec || attr->tlength <= writable)
		return 0;
	return uint64_t(pa_bytes_to_usec(attr->tlength - writable, spec)) * 1000;
}

bool AudioHardwareStreamPAOutput::renderBlock()
{
	if (!m_running || !m_stream || !m_blockBytes)
		return false;

	const size_t writable = pa_stream_writable_size(m_stream);
	if (writable == size_t(-1) || writable < m_blockBytes)
		return false;

	AudioBufferList* abl = static_cast<AudioBufferList*>(alloca(sizeof(AudioBufferList) + sizeof(AudioBuffer)));
	// Non-interleaved (planar) audio would have multiple buffers, but PA doesn't even support that AFAIK
	abl->mNumberBuffers = 1;
	abl->mBuffers[0].mNumberChannels = m_hw->asbd().mChannelsPerFrame;

	size_t rqsize = m_blockBytes;
	if (pa_stream_begin_write(m_stream, (void**) &abl->mBuffers[0].mData, &rqsize) < 0 || !rqsize)
		return false;
	if (rqsize < m_blockBytes)
	{
		pa_stream_cancel_write(m_stream);
		return false;
	}
	abl->mBuffers[0].mDataByteSize = m_blockBytes;

	const uint64_t latencyNs = outputLatencyNs(writable);
	const double rate = m_hw->asbd().mSampleRate;
	const double latencyFrames = rate > 0 ? double(latencyNs) * rate / 1e9 : 0;
	const uint64_t nowHost = mach_absolute_time();

	AudioTimeStamp now = {0};
	now.mHostTime = nowHost;
	// The position being played now: the block about to be rendered starts latencyFrames after it.
	now.mSampleTime = std::max(0.0, double(m_frames) - latencyFrames);
	now.mRateScalar = 1.0;
	now.mFlags = kAudioTimeStampHostTimeValid | kAudioTimeStampSampleTimeValid | kAudioTimeStampRateScalarValid;

	AudioTimeStamp output = now;
	output.mHostTime = nowHost + AudioConvertNanosToHostTime(latencyNs);
	output.mSampleTime = double(m_frames);

	// Call the client for more data
	OSStatus status = m_callback(m_hw->id(), &now, nullptr, nullptr, abl, &output, m_clientData);

	if (!m_running)
	{
		// The client stopped the stream from its IOProc.
		pa_stream_cancel_write(m_stream);
		return false;
	}
	if (status != noErr || !abl->mBuffers[0].mDataByteSize)
	{
		pa_stream_cancel_write(m_stream);
		pa_stream_cork(m_stream, true, [](pa_stream*, int, void*) {}, nullptr);
		return false;
	}

	if (m_convertSignedUnsigned)
		transformSignedUnsigned(abl);

	if (pa_stream_write(m_stream, abl->mBuffers[0].mData, m_blockBytes, nullptr, 0, PA_SEEK_RELATIVE) != 0)
		return false;

	m_frames += m_blockFrames;
	return true;
}

void AudioHardwareStreamPAOutput::paStreamWriteCB(pa_stream* s, size_t length, void* self)
{
	AudioHardwareStreamPAOutput* This = static_cast<AudioHardwareStreamPAOutput*>(self);

	if (!This->m_running)
	{
		pa_stream_cork(This->m_stream, true, [](pa_stream*, int, void*) {}, nullptr);
		return;
	}

	while (This->renderBlock())
		;
}

void AudioHardwareStreamPAOutput::start()
{
	struct pa_buffer_attr battr;

	// Size the server buffer in whole IOProc blocks, so that every request can be served by one.
	const uint32_t bpf = m_hw->asbd().mBytesPerFrame;
	uint32_t block = bpf ? (m_hw->bufferSize() / bpf) * bpf : m_bufferSize;
	if (!block)
		block = m_bufferSize;

	battr.fragsize = uint32_t(-1);
	battr.maxlength = block * 2 * kTargetBlocks;
	battr.minreq = block;
	// Starting before the buffer is full makes the server play past the data that has arrived
	// and drop the blocks written for that time, so playback waits for the whole target (the default).
	battr.prebuf = uint32_t(-1);
	battr.tlength = block * kTargetBlocks;

	m_blockBytes = block;
	m_blockFrames = bpf ? block / bpf : 0;

	pa_stream_set_write_callback(m_stream, paStreamWriteCB, this);

	pa_stream_connect_playback(m_stream, static_cast<AudioHardwareImplPA*>(m_hw)->paDeviceName(), &battr, pa_stream_flags_t(PA_STREAM_INTERPOLATE_TIMING
                                 |PA_STREAM_ADJUST_LATENCY
                                 |PA_STREAM_AUTO_TIMING_UPDATE) /* PA_STREAM_START_CORKED */,
			nullptr, nullptr);

	AudioHardwareStreamPA::start();
}
