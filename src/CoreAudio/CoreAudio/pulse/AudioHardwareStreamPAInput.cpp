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

AudioHardwareStreamPAInput::AudioHardwareStreamPAInput(AudioHardwareImplPA* hw, AudioDeviceIOProc callback, void* clientData)
: AudioHardwareStreamPA(hw, callback, clientData)
{
	// m_dump.open("/tmp/dump.raw", std::ios_base::out | std::ios_base::binary);
}

void AudioHardwareStreamPAInput::paStreamReadCB(pa_stream* s, size_t length, void* self)
{
	AudioHardwareStreamPAInput* This = static_cast<AudioHardwareStreamPAInput*>(self);

	std::unique_lock<std::mutex> l(This->m_stopMutex);

	if (!This->m_running)
		return;

	AudioTimeStamp fake = {0};
	AudioBufferList* abl = static_cast<AudioBufferList*>(alloca(sizeof(AudioBufferList) + sizeof(AudioBuffer)));
	std::vector<uint8_t> hole;

	// std::cout << "AudioHardwareStreamPAInput::paStreamReadCB()\n";

	while (true)
	{
		abl->mNumberBuffers = 1;
		abl->mBuffers[0].mNumberChannels = 2;

		size_t nbytes;

		int rv = pa_stream_peek(This->m_stream, (const void**) &abl->mBuffers[0].mData, &nbytes);

		if (rv < 0 || !nbytes)
		{
			// std::cout << "AudioHardwareStreamPAInput::paStreamReadCB(): peek rv=" << rv << ", nbytes=" << nbytes << std::endl;
			break;
		}

		if (nbytes && !abl->mBuffers[0].mData)
		{
			// "If there is a hole, data will be NULL and nbytes will contain the length of the hole."
			hole.resize(nbytes);
			abl->mBuffers[0].mData = hole.data();
		}
		abl->mBuffers[0].mDataByteSize = nbytes;

		// This->m_dump.write((char*) abl->mBuffers[0].mData, nbytes);
		// This->m_dump.flush();
		if (This->m_convertSignedUnsigned)
			This->transformSignedUnsigned(abl);

		// std::cout << "AudioHardwareStreamPAInput::paStreamReadCB(): bytes=" << nbytes << std::endl;
		OSStatus status = This->m_callback(This->m_hw->id(), &fake, abl, &fake, nullptr, nullptr, This->m_clientData);

		pa_stream_drop(This->m_stream);

		if (status != noErr)
		{
			pa_stream_cork(This->m_stream, true, [](pa_stream*, int, void*) {}, nullptr);
			break;
		}
	}
}

void AudioHardwareStreamPAInput::start()
{
	struct pa_buffer_attr battr;

	battr.fragsize = uint32_t(-1);
	battr.maxlength = uint32_t(m_bufferSize);
	battr.minreq = uint32_t(-1);
	battr.prebuf = battr.maxlength;
	battr.tlength = uint32_t(-1);

	// std::cout << "AudioHardwareStreamPAInput::start()\n";

	pa_stream_set_read_callback(m_stream, paStreamReadCB, this);

	int rv = pa_stream_connect_record(m_stream, nullptr, &battr,
		pa_stream_flags_t(PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_ADJUST_LATENCY | PA_STREAM_AUTO_TIMING_UPDATE)
	);

	// std::cout << "AudioHardwareStreamPAInput::start() rv = " << rv << std::endl;
	AudioHardwareStreamPA::start();
}
