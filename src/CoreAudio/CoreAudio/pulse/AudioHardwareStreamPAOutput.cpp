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

AudioHardwareStreamPAOutput::AudioHardwareStreamPAOutput(AudioDeviceIOProc callback, void* clientData)
: AudioHardwareStreamPA(callback, clientData)
{
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

		// pa_stream_set_state_callback(m_stream, paStreamStateCB, this);
		pa_stream_set_write_callback(m_stream, paStreamWriteCB, this);

		start();
	});
}

AudioHardwareStreamPAOutput::~AudioHardwareStreamPAOutput()
{
	if (m_stream)
		pa_stream_unref(m_stream);
}

void AudioHardwareStreamPAOutput::paStreamWriteCB(pa_stream* s, size_t length, void* self)
{
	AudioHardwareStreamPAOutput* This = static_cast<AudioHardwareStreamPAOutput*>(self);

	// TODO: call the client for more data
	// pa_stream_write(m_stream, ...);
}

void AudioHardwareStreamPAOutput::start()
{
	pa_stream_connect_playback(m_stream, nullptr, nullptr, (pa_stream_flags_t) 0 /* PA_STREAM_START_CORKED */,
			nullptr, nullptr);

	// pa_stream_cork(m_stream, false, [](pa_stream*, int, void*) {}, nullptr);
}

void AudioHardwareStreamPAOutput::stop(void(^cbDone)())
{
	m_cbDone = Block_copy(cbDone);

	pa_stream_cork(m_stream, true, [](pa_stream*, int, void* self) {
		AudioHardwareStreamPAOutput* This = static_cast<AudioHardwareStreamPAOutput*>(self);

		pa_stream_disconnect(This->m_stream);
		This->m_cbDone();

		Block_release(This->m_cbDone);
	}, this);
}
