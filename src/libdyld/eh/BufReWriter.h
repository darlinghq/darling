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

#ifndef BUFREWRITER_H
#define BUFREWRITER_H
#include "BufReader.h"

// only designed to rewrite ULEB128 values!
class BufReWriter : public BufReader
{
public:
	BufReWriter(void* mem, uintptr_t length);
	
	uint8_t read();
	uint64_t readULEB128();
	void updateLastULEB128(uint64_t value);
	void updateLast(uint8_t value);
private:
	uint8_t *m_pLastULEB128, *m_pLast8;
};

#endif
