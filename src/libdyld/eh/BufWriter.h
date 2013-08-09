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

#ifndef BUFWRITER_H
#define BUFWRITER_H
#include <stdint.h>
#include <stddef.h>
#include "DwarfPointer.h"

class BufWriter
{
public:
	BufWriter(void* mem, uintptr_t length);
	
	template<typename T> void writeType(T t) { writeBlock(&t, sizeof(T)); }
	
	void writePtr(uintptr_t v) { return writeType<uintptr_t>(v); }
	void writePtrS(intptr_t v) { return writeType<intptr_t>(v); }
	
	void write16(uint16_t v) { return writeType<uint16_t>(v); }
	void write16S(int16_t v) { return writeType<int16_t>(v); }
	
	void write32(uint32_t v) { return writeType<uint32_t>(v); }
	void write32S(int32_t v) { return writeType<int32_t>(v); }
	
	void write64(uint64_t v) { return writeType<uint64_t>(v); }
	void write64S(int64_t v) { return writeType<int64_t>(v); }
	
	void write(uint8_t v) { return writeType<uint8_t>(v); }
	void writeS(int8_t v) { return writeType<int8_t>(v); }
	
	void writeULEB128(uint64_t v);
	void writeLEB128(int64_t v);
	
	void writeDwarfPointer(DwarfPointer ptr);
	
	void writeBlock(const void* p, size_t bytes);
	void writeString(const char* str);
	
	bool atEnd() const { return m_pPos == m_pEnd; }
	uintptr_t pos() const { return reinterpret_cast<uintptr_t>(m_pPos); }
	uintptr_t relativePos() const { return m_pPos - m_pStart; }
	char* posPtr() const { return m_pPos; }
	void moveTo(uintptr_t pos) { m_pPos = reinterpret_cast<char*>(pos); }
private:
	void checkWrite(size_t bytes) const;
private:
	char *m_pStart, *m_pPos, *m_pEnd;
};

#endif
