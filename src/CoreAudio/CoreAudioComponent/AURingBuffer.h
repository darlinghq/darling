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

#ifndef AURINGBUFFER_H
#define AURINGBUFFER_H
#include <atomic>
#include <cstddef>
#include <vector>

// Single-producer/single-consumer ring buffer for interleaved float audio.
// Used by AUHAL to decouple the HAL's IOProc callback rate from the AU
// render-pull rate, replacing the previous single-buffer mutex/condvar
// handoff which could only hold exactly one buffer at a time.
class AURingBuffer
{
public:
	AURingBuffer(size_t capacityFrames, unsigned channels);

	// Returns the number of frames actually written (<= capacityFrames,
	// caps silently rather than blocking or growing).
	size_t write(const float* interleaved, size_t frames);

	// Returns the number of frames actually read (<= frames requested,
	// 0 if nothing is available). Does not zero-fill short reads --
	// callers fill the remainder with silence themselves.
	size_t read(float* interleaved, size_t frames);

	size_t framesAvailable() const;
	size_t capacityFrames() const { return m_capacityFrames; }

private:
	size_t m_capacityFrames;
	unsigned m_channels;
	std::vector<float> m_data; // capacityFrames * channels
	std::atomic<size_t> m_writePos{0}; // in frames, monotonically increasing
	std::atomic<size_t> m_readPos{0};
};

#endif /* AURINGBUFFER_H */
