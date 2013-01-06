/*
This file is part of Darling.

Copyright (C) 2013 Lubos Dolezel

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

#include "BufReader.h"
#include <stdexcept>

BufReader::BufReader(const void* mem, uintptr_t length)
{
	m_pPos = m_pStart = static_cast<const char*>(mem);
	m_pEnd = m_pStart + length;
}

void BufReader::checkRead(size_t bytes) const
{
	if (m_pPos+bytes > m_pEnd)
		throw std::runtime_error("Read beyond buffer's end");
}

const void* BufReader::readBlock(size_t bytes)
{
	const char* rv = m_pPos;
	
	checkRead(bytes);
	m_pPos += bytes;
	
	return rv;
}

const char* BufReader::readString()
{
	const char* start = m_pPos;
	
	while (read());
	
	return start;
}

void BufReader::unget(size_t bytes)
{
	if (m_pPos - bytes < m_pStart)
		throw std::runtime_error("Unget beyond buffer's start");
	m_pPos -= bytes;
}

uint64_t BufReader::readULEB128()
{
	uint64_t r = 0;
	int s = 0;
	uint8_t val;
	
	do
	{
		val = read();
		r |= (uint64_t)(val & 0x7f) << s;
		s += 7;
	}
	while (val & 0x80);
	
	return r;
}

int64_t BufReader::readLEB128()
{
	int64_t r = 0;
	int s = 0;
	for (;;)
	{
		uint8_t b = read();
		if (b < 0x80)
		{
			if (b & 0x40)
				r -= (0x80 - b) << s;
			else
				r |= (b & 0x3f) << s;
			break;
		}
		r |= (b & 0x7f) << s;
		s += 7;
	}
	return r;
}

DwarfPointer BufReader::readDwarfPointer(uint8_t encoding)
{
	DwarfPointer ptr;
	
	ptr.encoding = encoding;
	ptr.originalLocation = pos();
	
	switch (encoding & 0x0f)
	{
		case 0x01: // uleb128
			ptr.udata8 = readULEB128();
			break;
		case 0x02: // udata2
			ptr.udata2 = read16();
			break;
		case 0x03: // udata4
			ptr.udata4 = read32();
			break;
		case 0x04: // udata8
			ptr.udata8 = read64();
			break;
		case (0x01|0x08): // sleb128
			ptr.sdata8 = readLEB128();
			break;
		case (0x02|0x08): // sdata2
			ptr.sdata2 = read16S();
			break;
		case (0x03|0x08): // sdata4
			ptr.sdata4 = read32S();
			break;
		case (0x04|0x08): // sdata8
			ptr.sdata8 = read64S();
			break;
		case 0: // absptr
		{
			if (sizeof(void*) == 4)
				ptr.udata4 = read32();
			else
				ptr.udata8 = read64();
		}
	}
	
	return ptr;
}
