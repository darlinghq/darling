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

#ifndef AUDIOHARDWARESTREAMPAOUTPUT_H
#define AUDIOHARDWARESTREAMPAOUTPUT_H
#include "AudioHardwareStreamPA.h"
#include <cstdint>

class AudioHardwareStreamPAOutput : public AudioHardwareStreamPA
{
public:
	AudioHardwareStreamPAOutput(AudioHardwareImplPA* hw, AudioDeviceIOProc callback, void* clientData);

	// Frames PulseAudio holds in addition to the block an IOProc is rendering,
	// for an IOProc buffer of blockFrames.
	static uint32_t queuedFrames(uint32_t blockFrames);
private:
	void start() override;
	static void paStreamWriteCB(pa_stream* s, size_t length, void* self);

	// Renders and submits one block; false once no further block can be written.
	bool renderBlock();
	uint64_t outputLatencyNs(size_t writable);

	// Only touched on the PA queue.
	uint64_t m_frames = 0;
	size_t m_blockBytes = 0;
	uint32_t m_blockFrames = 0;
};

#endif /* AUDIOHARDWARESTREAMPA_H */

