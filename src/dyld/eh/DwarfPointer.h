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

#ifndef DWARFPOINTER_H
#define DWARFPOINTER_H
#include <stdint.h>

struct DwarfPointer
{
	DwarfPointer() : encoding(0xff), sdata8(0) {}
	
	bool relocateToAddr(uintptr_t addr, uintptr_t originalBlockStart, uintptr_t originalBlockEnd);
	int64_t getSigned();
	void setSigned(intptr_t value);
	
	// Used to change the internal data type to what is used after relocation
	// Needed when we need to write the encoding somewhere (CIE) long before writing the pointer itself (FDE)
	void prepareForRelocation();
	int getSize();
	static int getSize(uint8_t encoding);
	
	uint8_t encoding;
	union
	{
		uint64_t udata8;
		int64_t sdata8;
		uint32_t udata4;
		int32_t sdata4;
		uint16_t udata2;
		int32_t sdata2;
	};
	
	uintptr_t originalLocation;
};

#endif
