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

AudioHardwareStreamPAOutput::AudioHardwareStreamPAOutput(AudioHardwareImpl* hw, AudioDeviceIOProc callback, void* clientData)
: AudioHardwareStreamPA(hw, callback, clientData)
{
	// TODO: Abstract this into our superclass
	AudioHardwareImplPA::getPAContext(^(pa_context* context) {
		if (!context)
		{
			std::cerr << "Failed to get PulseAudio context\n";
			return;
		}

		pa_sample_spec spec;

		spec.rate = 44100;
		spec.channels = 2;
#if defined(__BIG_ENDIAN__)
		spec.format = PA_SAMPLE_FLOAT32BE;
#elif defined(__LITTLE_ENDIAN__)
		spec.format = PA_SAMPLE_FLOAT32LE;
#else
#	error Unknown endianess!
#endif
		if (!pa_sample_spec_valid(&spec))
		{
			std::cerr << "Failed to create a valid pa_sample_spec\n";
			return;
		}

		m_stream = pa_stream_new(context, "CoreAudio", &spec, nullptr);

		//pa_stream_set_state_callback(m_stream, [](pa_stream *s, void *userdata) {
			//std::cout << "Stream state: " << pa_stream_get_state(s) << std::endl;
			/*if (pa_stream_get_state(s) == 2)
				pa_stream_trigger(s, [](pa_stream *s, int success, void *userdata){}, nullptr);*/
		//}, this);
		pa_stream_set_write_callback(m_stream, paStreamWriteCB, this);
		//pa_stream_set_underflow_callback(m_stream, [](pa_stream *s, void *userdata) {
		//	std::cerr << "Underflow!\n";
		//}, nullptr);

		start();
	});
}

AudioHardwareStreamPAOutput::~AudioHardwareStreamPAOutput()
{
	// std::cout << "AudioHardwareStreamPAOutput::~AudioHardwareStreamPAOutput()\n";
	if (m_stream)
		pa_stream_unref(m_stream);
}

void AudioHardwareStreamPAOutput::paStreamWriteCB(pa_stream* s, size_t length, void* self)
{
	// std::cout << "AudioHardwareStreamPAOutput::paStreamWriteCB()\n";
	AudioHardwareStreamPAOutput* This = static_cast<AudioHardwareStreamPAOutput*>(self);

	AudioTimeStamp fake = {0};
	AudioBufferList* abl = static_cast<AudioBufferList*>(alloca(sizeof(AudioBufferList) + sizeof(AudioBuffer)));

	// Non-interleaved (planar) audio would have multiple buffers, but PA doesn't even support that AFAIK
	abl->mNumberBuffers = 1;

	abl->mBuffers[0].mNumberChannels = 2;

	/*size_t nbytes = This->m_bufferSize;
	pa_stream_begin_write(This->m_stream, (void**) &abl->mBuffers[0].mData, &nbytes);
	abl->mBuffers[0].mDataByteSize = nbytes;*/

	size_t done = 0;

	while (done < length)
	{
		abl->mBuffers[0].mData = This->m_buffer;

		// std::cout << "AudioHardwareStreamPAOutput() length req=" << length << ", done=" << done << std::endl;
		size_t rqsize = std::min<UInt32>(This->m_bufferSize, length-done);
		abl->mBuffers[0].mDataByteSize = rqsize;

		// Call the client for more data
		OSStatus status = This->m_callback(This->m_hw->id(), &fake, nullptr, nullptr, abl, &fake, This->m_clientData);
		if (status != noErr || !abl->mBuffers[0].mDataByteSize)
		{
			// std::cerr << "AudioDeviceIOProc returned " << status << ", corking...\n";

			pa_stream_cork(This->m_stream, true, [](pa_stream*, int, void*) {}, nullptr);
		}
		else
		{
			// std::cout << "AudioHardwareStreamPAOutput::paStreamWriteCB(): got " << abl->mBuffers[0].mDataByteSize << " bytes\n";
			int rv = pa_stream_write(This->m_stream, abl->mBuffers[0].mData, abl->mBuffers[0].mDataByteSize, nullptr, 0, PA_SEEK_RELATIVE);
			if (rv != 0)
			{
				// std::cerr << "pa_stream_write() failed\n";
				break;
			}
			//else
			//	std::cout << "pa_stream_write() OK\n";
		}

		done += abl->mBuffers[0].mDataByteSize;
	}
}

void AudioHardwareStreamPAOutput::start()
{
	struct pa_buffer_attr battr;

	battr.fragsize = uint32_t(-1);
	battr.maxlength = uint32_t(m_bufferSize);
	battr.minreq = uint32_t(-1);
	battr.prebuf = battr.maxlength;
	battr.tlength = uint32_t(-1);

	pa_stream_connect_playback(m_stream, nullptr, &battr, pa_stream_flags_t(PA_STREAM_INTERPOLATE_TIMING
                                 |PA_STREAM_ADJUST_LATENCY
                                 |PA_STREAM_AUTO_TIMING_UPDATE) /* PA_STREAM_START_CORKED */,
			nullptr, nullptr);

	// pa_stream_cork(m_stream, false, [](pa_stream*, int, void*) {}, nullptr);
}

void AudioHardwareStreamPAOutput::stop(void(^cbDone)())
{
	// std::cerr << "AudioHardwareStreamPAOutput::stop()\n";
	m_cbDone = Block_copy(cbDone);

	pa_stream_cork(m_stream, true, [](pa_stream*, int, void* self) {
		AudioHardwareStreamPAOutput* This = static_cast<AudioHardwareStreamPAOutput*>(self);

		pa_stream_disconnect(This->m_stream);
		This->m_cbDone();

		Block_release(This->m_cbDone);
	}, this);
}
