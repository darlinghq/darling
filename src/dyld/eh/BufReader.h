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

#ifndef BUFREADER_H
#define BUFREADER_H
#include <stdint.h>
#include <stddef.h>
#include "DwarfPointer.h"


class BufReader
{
public:
	BufReader(const void* mem, uintptr_t length);
	
	template<typename T> T readType() { const T* t = static_cast<const T*>(readBlock(sizeof(T))); return *t; }
	
	uintptr_t readPtr() { return readType<uintptr_t>(); }
	intptr_t readPtrS() { return readType<intptr_t>(); }
	
	uint16_t read16() { return readType<uint16_t>(); }
	int16_t read16S() { return readType<int16_t>(); }
	
	uint32_t read32() { return readType<uint32_t>(); }
	int32_t read32S() { return readType<int32_t>(); }
	
	uint64_t read64() { return readType<uint64_t>(); }
	int64_t read64S() { return readType<int64_t>(); }
	
	uint8_t read() { return readType<uint8_t>(); }
	int8_t readS() { return readType<int8_t>(); }
	
	uint64_t readULEB128();
	int64_t readLEB128();
	
	DwarfPointer readDwarfPointer(uint8_t encoding);
	
	const void* readBlock(size_t bytes);
	const char* readString();
	
	void unget(size_t bytes);
	
	bool atEnd() const { return m_pPos == m_pEnd; }
	uintptr_t pos() const { return reinterpret_cast<uintptr_t>(m_pPos); }
	void moveTo(uintptr_t pos) { m_pPos = reinterpret_cast<const char*>(pos); }
private:
	void checkRead(size_t bytes) const;
private:
	const char *m_pStart, *m_pPos, *m_pEnd;
};

#endif
