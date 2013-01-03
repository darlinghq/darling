/*
This file is part of Darling.

Copyright (C) 2012-2013 Lubos Dolezel

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

#ifndef TOOLUTILS_H
#define TOOLUTILS_H

extern "C" {

int BitTst(const void* bytePtr, long bitNum);
void BitSet(void* bytePtr, long bitNum);
void BitClr(void* bytePtr, long bitNum);
long BitAnd(long v1, long v2);
long BitXor(long v1, long v2);
long BitNot(long v1);
long BitShift(long value, short count);

}

#endif

