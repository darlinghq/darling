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

#include "BufWriter.h"
#include <stdexcept>
#include <cstring>

BufWriter::BufWriter(void* mem, uintptr_t length)
{
	m_pPos = m_pStart = static_cast<char*>(mem);
	m_pEnd = m_pStart + length;
}

void BufWriter::writeULEB128(uint64_t v)
{
	do
	{
		uint8_t byte = v & 0x7f;
		v >>= 7;
		if (v != 0)
			byte |= 0x80;
		write(byte);
	}
	while (v != 0);
}

void BufWriter::writeLEB128(int64_t v)
{
	bool more = true;
	bool negative = (v < 0);
	
	int bitsUsed = 0;
	for (int i = 0; i < 64; i++)
	{
		if (v & (1ll << i))
			bitsUsed = i+1;
	}

	while(more)
	{
		uint8_t byte = v & 0x7f;
		v >>= 7;
		
		if (negative)
			v |= - (1 <<(bitsUsed - 7));

		if ((v == 0 && !(byte & 0x40)) || (v == -1 && (byte & 0x40)))
			more = false;
		else
			byte |= 0x80;
		write(byte);
	}
}

void BufWriter::writeDwarfPointer(DwarfPointer ptr)
{
	switch (ptr.encoding & 0xf)
	{
		case 0x01: // uleb128
			writeULEB128(ptr.udata8);
			break;
		case 0x02: // udata2
			write16(ptr.udata2);
			break;
		case 0x03: // udata4
			write32(ptr.udata4);
			break;
		case 0x04: // udata8
			write64(ptr.udata8);
			break;
		case (0x01|0x08): // sleb128
			writeLEB128(ptr.sdata8);
			break;
		case (0x02|0x08): // sdata2
			write16S(ptr.sdata2);
			break;
		case (0x03|0x08): // sdata4
			write32S(ptr.sdata4);
			break;
		case (0x04|0x08): // sdata8
			write64S(ptr.sdata8);
			break;
		case 0: // absptr
		{
			if (sizeof(void*) == 4)
				write32(ptr.udata4);
			else
				write64(ptr.udata8);
		}
	}
}

void BufWriter::writeBlock(const void* p, size_t bytes)
{
	checkWrite(bytes);
	memcpy(m_pPos, p, bytes);
	m_pPos += bytes;
}

void BufWriter::writeString(const char* str)
{
	writeBlock(str, strlen(str)+1);
}

void BufWriter::checkWrite(size_t bytes) const
{
	if (m_pPos+bytes > m_pEnd)
		throw std::runtime_error("Write beyond buffer's end");
}
