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
#include <type_traits>
#include <limits>

AudioHardwareStreamPA::AudioHardwareStreamPA(AudioHardwareImplPA* hw, AudioDeviceIOProc callback, void* clientData)
: AudioHardwareStream(hw, false), m_callback(callback), m_clientData(clientData)
{
}

void AudioHardwareStreamPA::initialize()
{
	AudioHardwareImplPA* hw = static_cast<AudioHardwareImplPA*>(m_hw);
	hw->getPAContext(^(pa_context* context) {
		if (!context)
		{
			std::cerr << "Failed to get PulseAudio context\n";
			return;
		}
		
		pa_sample_spec spec = AudioHardwareImplPA::paSampleSpecForASBD(hw->asbd(), &m_convertSignedUnsigned);
		
		if (!pa_sample_spec_valid(&spec))
		{
			std::cerr << "Failed to create a valid pa_sample_spec\n";
			return;
		}

		m_stream = pa_stream_new(context, "CoreAudio", &spec, nullptr);
		if (!m_stream)
			return;

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
	static_cast<AudioHardwareImplPA*>(m_hw)->syncPA(^{
		if (m_stream)
		{
			pa_stream_unref(m_stream);
			m_stream = nullptr;
		}
	});
}

void AudioHardwareStreamPA::start()
{
	m_running = true;
}

static void disconnectWhenReadyCB(pa_stream* stream, void*)
{
	switch (pa_stream_get_state(stream))
	{
		case PA_STREAM_READY:
			pa_stream_disconnect(stream);
			break;
		case PA_STREAM_FAILED:
		case PA_STREAM_TERMINATED:
			pa_stream_set_state_callback(stream, nullptr, nullptr);
			pa_stream_unref(stream);
			break;
		default:
			break;
	}
}

void AudioHardwareStreamPA::stop()
{
	static_cast<AudioHardwareImplPA*>(m_hw)->syncPA(^{
		// All stream operations and callbacks are serialized by syncPA, and an IOProc may
		// call stop() itself, so this must not take a lock the callbacks hold.
		m_running = false;
		if (m_stream)
		{
			// Disconnect may leave queued protocol work holding a stream ref.
			// Detach client callbacks before the C++ owner can be destroyed.
			pa_stream_set_write_callback(m_stream, nullptr, nullptr);
			pa_stream_set_read_callback(m_stream, nullptr, nullptr);
			pa_stream_set_state_callback(m_stream, nullptr, nullptr);

			if (pa_stream_get_state(m_stream) == PA_STREAM_CREATING)
			{
				// A stream can't be disconnected until the server has answered the connection
				// request. It finishes on its own, holding a reference, after this object is gone.
				pa_stream_ref(m_stream);
				pa_stream_set_state_callback(m_stream, disconnectWhenReadyCB, nullptr);
			}
			else
				pa_stream_disconnect(m_stream);
		}
	});
}

// This function seems to only convert unsigned to signed, but it works both ways in practice
template <typename T>
void transform(typename std::make_unsigned<T>::type* data)
{
	typedef typename std::make_signed<T>::type signed_type;
	signed_type* s = reinterpret_cast<signed_type*>(data);

	*s = *data + std::numeric_limits<signed_type>::min();
}

void AudioHardwareStreamPA::transformSignedUnsigned(AudioBufferList* abl) const
{
	const AudioStreamBasicDescription& asbd = m_hw->asbd();
	const bool revEndian = (asbd.mFormatFlags & kAudioFormatFlagIsBigEndian) != (asbd.mFormatFlags & kAudioFormatFlagsNativeEndian);

	for (int i = 0; i < abl->mNumberBuffers; i++)
	{
		AudioBuffer* buf = &abl->mBuffers[i];

		switch (asbd.mBitsPerChannel)
		{
			case 8:
				for (int j = 0; j < buf->mDataByteSize; j++)
					transform<uint8_t>(reinterpret_cast<uint8_t*>(buf->mData) + j);
				break;
			case 16:
				if (!revEndian)
				{
					for (int j = 0; j < buf->mDataByteSize / sizeof(uint16_t); j++)
						transform<uint16_t>(reinterpret_cast<uint16_t*>(buf->mData) + j);
				}
				else
				{
					for (int j = 0; j < buf->mDataByteSize / sizeof(uint16_t); j++)
					{
						uint16_t v = __builtin_bswap16(*(reinterpret_cast<uint16_t*>(buf->mData) + j));
						transform<uint16_t>(&v);
						*(reinterpret_cast<uint16_t*>(buf->mData) + j) = __builtin_bswap16(v);
					}
				}
				break;
			case 24:
				for (int j = 0; j < buf->mDataByteSize / sizeof(uint32_t); j++)
				{
					uint32_t v = *(reinterpret_cast<uint32_t*>(buf->mData) + j);
					if (revEndian)
						v = __builtin_bswap32(v);
					
					// sign extend
					if (v & 0x800000)
						v |= 0xff000000;

					transform<uint32_t>(&v);
					
					v &= 0xffffff;
					if (revEndian)
						v = __builtin_bswap32(v);
					
					*(reinterpret_cast<uint32_t*>(buf->mData) + j) = v;
				}
				break;
			case 32:
				if (!revEndian)
				{
					for (int j = 0; j < buf->mDataByteSize / sizeof(uint32_t); j++)
						transform<uint32_t>(reinterpret_cast<uint32_t*>(buf->mData) + j);
				}
				else
				{
					for (int j = 0; j < buf->mDataByteSize / sizeof(uint32_t); j++)
					{
						uint32_t v = __builtin_bswap32(*(reinterpret_cast<uint32_t*>(buf->mData) + j));
						transform<uint32_t>(&v);
						*(reinterpret_cast<uint32_t*>(buf->mData) + j) = __builtin_bswap32(v);
					}
				}
				break;
		}
	}
}
