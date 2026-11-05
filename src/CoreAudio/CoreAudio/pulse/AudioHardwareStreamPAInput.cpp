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

#include "AudioHardwareStreamPAInput.h"
#include "AudioHardwareImplPA.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <mach/mach_time.h>
#include "CoreAudio/HostTime.h"

AudioHardwareStreamPAInput::AudioHardwareStreamPAInput(AudioHardwareImplPA* hw, AudioDeviceIOProc callback, void* clientData)
: AudioHardwareStreamPA(hw, callback, clientData)
{
	initialize();
	// m_dump.open("/tmp/dump.raw", std::ios_base::out | std::ios_base::binary);
}

void AudioHardwareStreamPAInput::paStreamReadCB(pa_stream* s, size_t length, void* self)
{
	AudioHardwareStreamPAInput* This = static_cast<AudioHardwareStreamPAInput*>(self);


	if (!This->m_running)
		return;

	const pa_sample_spec* spec = pa_stream_get_sample_spec(This->m_stream);
	const size_t frameBytes = spec ? pa_frame_size(spec) : 0;
	const double rate = spec ? double(spec->rate) : 0;
	if (!frameBytes || rate <= 0)
		return;

	AudioBufferList* abl = static_cast<AudioBufferList*>(alloca(sizeof(AudioBufferList) + sizeof(AudioBuffer)));

	// std::cout << "AudioHardwareStreamPAInput::paStreamReadCB()\n";

	while (true)
	{
		const void* data = nullptr;
		size_t nbytes = 0;

		int rv = pa_stream_peek(This->m_stream, &data, &nbytes);

		if (rv < 0 || !nbytes)
		{
			// std::cout << "AudioHardwareStreamPAInput::paStreamReadCB(): peek rv=" << rv << ", nbytes=" << nbytes << std::endl;
			break;
		}

		// The fragment at the read index was captured `latency` ago: source latency plus
		// everything still unread. Without timing info yet, count only the unread data.
		pa_usec_t usec = 0;
		int negative = 0;
		uint64_t latencyNs;
		if (pa_stream_get_latency(This->m_stream, &usec, &negative) == 0)
			latencyNs = negative ? 0 : uint64_t(usec) * 1000;
		else
		{
			size_t readable = pa_stream_readable_size(This->m_stream);
			latencyNs = uint64_t(pa_bytes_to_usec(std::max(readable, nbytes), spec)) * 1000;
		}
		const uint64_t nowHost = mach_absolute_time();

		const bool hole = !data;
		if (hole || This->m_convertSignedUnsigned)
		{
			// "If there is a hole, data will be NULL and nbytes will contain the length of the hole."
			// Holes are delivered as silence so the sample clock keeps counting real time.
			// The peeked memory belongs to PulseAudio, so convert a private copy.
			This->m_scratch.resize(nbytes);
			if (data)
				memcpy(This->m_scratch.data(), data, nbytes);
			else
				memset(This->m_scratch.data(), spec->format == PA_SAMPLE_U8 ? 0x80 : 0, nbytes);
			data = This->m_scratch.data();
		}

		// Like a real HAL, hand the IOProc at most one device buffer per call.
		const size_t block = std::max(frameBytes, This->m_blockBytes);
		for (size_t offset = 0; offset + frameBytes <= nbytes; )
		{
			size_t piece = std::min(block, nbytes - offset);
			piece -= piece % frameBytes;

			abl->mNumberBuffers = 1;
			abl->mBuffers[0].mNumberChannels = spec->channels;
			abl->mBuffers[0].mData = const_cast<uint8_t*>(static_cast<const uint8_t*>(data) + offset);
			abl->mBuffers[0].mDataByteSize = piece;

			if (This->m_convertSignedUnsigned)
				This->transformSignedUnsigned(abl);

			const uint64_t offsetNs = uint64_t(double(offset / frameBytes) * 1e9 / rate);
			const uint64_t agoNs = latencyNs > offsetNs ? latencyNs - offsetNs : 0;
			uint64_t inputHost = nowHost - std::min(nowHost, AudioConvertNanosToHostTime(agoNs));
			// Keep host time monotonic even when PA's latency estimate jumps.
			if (inputHost <= This->m_lastInputHost)
				inputHost = This->m_lastInputHost + 1;
			This->m_lastInputHost = inputHost;

			AudioTimeStamp input = {0};
			input.mSampleTime = double(This->m_frames);
			input.mHostTime = inputHost;
			input.mRateScalar = 1.0;
			input.mFlags = kAudioTimeStampHostTimeValid | kAudioTimeStampSampleTimeValid | kAudioTimeStampRateScalarValid;

			AudioTimeStamp now = input;
			now.mHostTime = std::max(nowHost, inputHost);
			now.mSampleTime = input.mSampleTime + double(AudioConvertHostTimeToNanos(now.mHostTime - inputHost)) * rate / 1e9;

			// This->m_dump.write((char*) abl->mBuffers[0].mData, nbytes);
			// This->m_dump.flush();
			// A real HAL ignores the IOProc's result; one failed callback must not
			// silence the microphone for the rest of the session.
			This->m_callback(This->m_hw->id(), &now, abl, &input, nullptr, nullptr, This->m_clientData);

			if (!This->m_running) return;
			This->m_frames += piece / frameBytes;
			offset += piece;
		}

		pa_stream_drop(This->m_stream);
	}
}

void AudioHardwareStreamPAInput::start()
{
	struct pa_buffer_attr battr;

	// Record streams only use maxlength and fragsize. Ask for fragments of one device buffer
	// (kAudioDevicePropertyBufferFrameSize) for voice latency, and bound the server-side
	// backlog so a stalled client can't accumulate seconds of delay.
	const uint32_t bpf = m_hw->asbd().mBytesPerFrame;
	uint32_t block = bpf ? (m_hw->bufferSize() / bpf) * bpf : m_bufferSize;
	if (!block)
		block = m_bufferSize;
	m_blockBytes = block;

	battr.fragsize = block;
	battr.maxlength = block * 16;
	battr.minreq = uint32_t(-1);
	battr.prebuf = uint32_t(-1);
	battr.tlength = uint32_t(-1);

	// std::cout << "AudioHardwareStreamPAInput::start()\n";

	pa_stream_set_read_callback(m_stream, paStreamReadCB, this);

	int rv = pa_stream_connect_record(m_stream, static_cast<AudioHardwareImplPA*>(m_hw)->paDeviceName(), &battr,
		pa_stream_flags_t(PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_ADJUST_LATENCY | PA_STREAM_AUTO_TIMING_UPDATE)
	);
	// std::cout << "AudioHardwareStreamPAInput::start() rv = " << rv << std::endl;
	AudioHardwareStreamPA::start();
}
