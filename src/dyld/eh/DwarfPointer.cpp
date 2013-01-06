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

#include "DwarfPointer.h"
#include <stdexcept>

bool DwarfPointer::relocateToAddr(uintptr_t addr, uintptr_t originalBlockStart, uintptr_t originalBlockEnd)
{
	if (!(encoding & 0x10)) // DW_EH_PE_pcrel; == cannot be used because pcrel can be combined with indirect...
		return false;
	
	if (originalLocation+getSigned() >= originalBlockStart && originalLocation+getSigned() < originalBlockEnd)
		return false; // nothing to change, it was inside of the EH block
	
	setSigned(getSigned() + (originalLocation - addr));
	originalLocation = addr;
	return true;
}

int DwarfPointer::getSize()
{
	return getSize(encoding);
}

int DwarfPointer::getSize(uint8_t encoding)
{
	switch (encoding & 0xf)
	{
		case 0x01: // uleb128
			throw std::logic_error("leb128 size cannot be precalculated");
		case 0x02: // udata2
			return 2;
		case 0x03: // udata4
			return 4;
		case 0x04: // udata8
			return 8;
		case (0x01|0x08): // sleb128
			throw std::logic_error("uleb128 size cannot be precalculated");
		case (0x02|0x08): // sdata2
			return 2;
		case (0x03|0x08): // sdata4
			return 4;
		case (0x04|0x08): // sdata8
			return 8;
		case 0: // absptr
			return sizeof(void*);
	}
	return 0;
}

int64_t DwarfPointer::getSigned()
{
	switch (encoding & 0xf)
	{
		case 0x01: // uleb128
			return udata8;
		case 0x02: // udata2
			return udata2;
		case 0x03: // udata4
			return udata4;
		case 0x04: // udata8
			return udata8;
		case (0x01|0x08): // sleb128
			return sdata8;
		case (0x02|0x08): // sdata2
			return sdata2;
		case (0x03|0x08): // sdata4
			return sdata4;
		case (0x04|0x08): // sdata8
			return sdata8;
		case 0: // absptr
		{
			if (sizeof(void*) == 4)
				return udata4;
			else
				return udata8;
		}
	}
	return 0; // to shut the compiler up
}

void DwarfPointer::setSigned(intptr_t value)
{
	uint8_t newEncoding = encoding & 0xf0;
	
	// always use sdata4/8 to simplify calculations in EHSection
	if (sizeof(void*) == 8)
	{
		newEncoding |= (0x04|0x08);
		sdata8 = value;
	}
	else
	{
		newEncoding |= (0x03|0x08);
		sdata4 = value;
	}
	encoding = newEncoding;
}

void DwarfPointer::prepareForRelocation()
{
	if (encoding != 0xff) // DW_EH_PE_omit
		setSigned(getSigned());
}
