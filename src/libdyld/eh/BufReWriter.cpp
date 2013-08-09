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

#include "BufReWriter.h"
#include <cassert>
#include <stdexcept>

BufReWriter::BufReWriter(void* mem, uintptr_t length)
: BufReader(mem, length), m_pLastULEB128(nullptr), m_pLast8(nullptr)
{
}
	
uint64_t BufReWriter::readULEB128()
{
	m_pLastULEB128 = (uint8_t*) m_pPos;
	return BufReader::readULEB128();
}

uint8_t BufReWriter::read()
{
	m_pLast8 = (uint8_t*) m_pPos;
	return BufReader::read();
}

void BufReWriter::updateLastULEB128(uint64_t v)
{
	assert(m_pLastULEB128 != nullptr);
	
	uint8_t* pos = m_pLastULEB128;
	
	do
	{
		uint8_t byte = v & 0x7f;
		v >>= 7;
		if (v != 0)
			byte |= 0x80;
		
		if (!(*pos & 0x80) && v != 0)
			std::runtime_error("New ULEB128 value didn't fit into the space of the previous value");
		
		*pos = byte;
		pos++;
	}
	while (v != 0);
}

void BufReWriter::updateLast(uint8_t value)
{
	assert(m_pLast8 != nullptr);
	*m_pLast8 = value;
}
