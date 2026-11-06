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

#include "AURingBuffer.h"
#include <algorithm>
#include <cstring>

AURingBuffer::AURingBuffer(size_t capacityFrames, unsigned channels, unsigned bytesPerSample)
: m_capacityFrames(capacityFrames)
, m_frameBytes(size_t(channels) * bytesPerSample)
, m_data(capacityFrames * m_frameBytes, 0)
{
}

size_t AURingBuffer::write(const void* interleaved, size_t frames)
{
	const uint8_t* source = static_cast<const uint8_t*>(interleaved);
	size_t writePos = m_writePos.load(std::memory_order_relaxed);
	size_t readPos = m_readPos.load(std::memory_order_acquire);
	size_t free = m_capacityFrames - (writePos - readPos);

	size_t toWrite = std::min(frames, free);
	for (size_t i = 0; i < toWrite; i++)
	{
		size_t slot = (writePos + i) % m_capacityFrames;
		std::memcpy(&m_data[slot * m_frameBytes], source + i * m_frameBytes, m_frameBytes);
	}

	m_writePos.store(writePos + toWrite, std::memory_order_release);
	return toWrite;
}

size_t AURingBuffer::read(void* interleaved, size_t frames)
{
	uint8_t* destination = static_cast<uint8_t*>(interleaved);
	size_t writePos = m_writePos.load(std::memory_order_acquire);
	size_t readPos = m_readPos.load(std::memory_order_relaxed);
	size_t available = writePos - readPos;

	size_t toRead = std::min(frames, available);
	for (size_t i = 0; i < toRead; i++)
	{
		size_t slot = (readPos + i) % m_capacityFrames;
		std::memcpy(destination + i * m_frameBytes, &m_data[slot * m_frameBytes], m_frameBytes);
	}

	m_readPos.store(readPos + toRead, std::memory_order_release);
	return toRead;
}

size_t AURingBuffer::framesAvailable() const
{
	return m_writePos.load(std::memory_order_acquire) - m_readPos.load(std::memory_order_relaxed);
}
