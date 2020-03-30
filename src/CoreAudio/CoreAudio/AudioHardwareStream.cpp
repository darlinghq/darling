/*
This file is part of Darling.

Copyright (C) 2015-2020 Lubos Dolezel

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

#include "AudioHardwareStream.h"
#include "AudioHardwareImpl.h"
#include <condition_variable>
#include <mutex>
#include <iostream>

AudioHardwareStream::AudioHardwareStream(AudioHardwareImpl* hw, bool needBuffer)
: m_hw(hw)
{
	m_bufferSize = hw->bufferSize();

	if (needBuffer)
		m_buffer = new uint8_t[m_bufferSize];
	else
		m_buffer = nullptr;
}

AudioHardwareStream::~AudioHardwareStream()
{
	delete m_buffer;
}
