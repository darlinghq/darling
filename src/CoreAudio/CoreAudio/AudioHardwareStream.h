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

#ifndef AUDIOHARDWARESTREAM_H
#define AUDIOHARDWARESTREAM_H
#include <stdint.h>

class AudioHardwareImpl;

class AudioHardwareStream
{
public:
	AudioHardwareStream(AudioHardwareImpl* hw, bool needBuffer = true);
	virtual ~AudioHardwareStream();

	virtual void stop(/*void(^cbDone)()*/) = 0;
	//void stop();
protected:
	AudioHardwareImpl* m_hw;
	uint32_t m_bufferSize;
	uint8_t* m_buffer;
};

#endif /* AUDIOHARDWARESTREAM_H */

