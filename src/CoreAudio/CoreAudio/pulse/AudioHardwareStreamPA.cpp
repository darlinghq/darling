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

#include "AudioHardwareStreamPA.h"
#include "AudioHardwareImplPA.h"
#include <iostream>

AudioHardwareStreamPA::AudioHardwareStreamPA(AudioHardwareImpl* hw, AudioDeviceIOProc callback, void* clientData)
: AudioHardwareStream(hw, false), m_callback(callback), m_clientData(clientData)
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

		//pa_stream_set_state_callback(m_stream, [](pa_stream *s, void *userdata) {
			//std::cout << "Stream state: " << pa_stream_get_state(s) << std::endl;
			/*if (pa_stream_get_state(s) == 2)
				pa_stream_trigger(s, [](pa_stream *s, int success, void *userdata){}, nullptr);*/
		//}, this);
		
		//pa_stream_set_underflow_callback(m_stream, [](pa_stream *s, void *userdata) {
		//	std::cerr << "Underflow!\n";
		//}, nullptr);

		start();
	});
}

AudioHardwareStreamPA::~AudioHardwareStreamPA()
{
	if (m_stream)
		pa_stream_unref(m_stream);
}

void AudioHardwareStreamPA::stop(void(^cbDone)())
{
	// std::cerr << "AudioHardwareStreamPA::stop()\n";
	m_cbDone = Block_copy(cbDone);

	pa_stream_cork(m_stream, true, [](pa_stream*, int, void* self) {
		AudioHardwareStreamPA* This = static_cast<AudioHardwareStreamPA*>(self);

		pa_stream_disconnect(This->m_stream);
		This->m_cbDone();

		Block_release(This->m_cbDone);
	}, this);
}

