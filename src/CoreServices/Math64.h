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

#ifndef MATH64_H
#define MATH64_H

#include "MacTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

int64_t S64Max();
int64_t S64Min();

int64_t S64Add(int64_t v1, int64_t v2);
int64_t S64Subtract(int64_t v1, int64_t v2);
int64_t S64Negate(int64_t v);
int64_t S64Absolute(int64_t v);
int64_t S64Multiply(int64_t v1, int64_t v2);
int64_t S64Mod(int64_t divd, int64_t divs);
int64_t S64Divide(int64_t divd, int64_t divs, int64_t* rem);
int64_t S64Div(int64_t divd, int64_t divs);
int64_t S64Set(int32_t v);
int64_t S64SetU(uint32_t v);
int32_t S32Set(int64_t v);
int S64And(int64_t v1, int64_t v2);
int S64Or(int64_t v1, int64_t v2);
int S64Eor(int64_t v1, int64_t v2);
int S64Not(int64_t v);
int32_t S64Compare(int64_t l, int64_t r);
int64_t S64BitwiseAnd(int64_t v1, int64_t v2);
int64_t S64BitwiseOr(int64_t v1, int64_t v2);
int64_t S64BitwiseEor(int64_t v1, int64_t v2);
int64_t S64BitwiseNor(int64_t v1);
int64_t S64ShiftRight(int64_t v, uint32_t shift);
int64_t S64ShiftLeft(int64_t v, uint32_t shift);
long double S64ToLongDouble(int64_t v);
int64_t LongDoubleToSInt64(long double v);

uint64_t U64Max();
uint64_t U64Add(uint64_t v1, uint64_t v2);
uint64_t U64Subtract(uint64_t v1, uint64_t v2);
uint64_t U64Multiply(uint64_t v1, uint64_t v2);
uint64_t U64Mod(uint64_t divd, uint64_t divs);
uint64_t U64Divide(uint64_t divd, uint64_t divs, uint64_t* rem);
uint64_t U64Div(uint64_t divd, uint64_t divs);
uint64_t U64Set(int32_t v);
uint64_t U64SetU(uint32_t v);
int U64And(uint64_t v1, uint64_t v2);
int U64Or(uint64_t v1, uint64_t v2);
int U64Eor(uint64_t v1, uint64_t v2);
int U64Not(uint64_t v);
int32_t U64Compare(uint64_t l, uint64_t r);
uint64_t U64BitwiseAnd(uint64_t v1, uint64_t v2);
uint64_t U64BitwiseOr(uint64_t v1, uint64_t v2);
uint64_t U64BitwiseEor(uint64_t v1, uint64_t v2);
uint64_t U64BitwiseNor(uint64_t v1);
uint64_t U64ShiftRight(uint64_t v, uint32_t shift);
uint64_t U64ShiftLeft(uint64_t v, uint32_t shift);
long double U64ToLongDouble(uint64_t v);
uint64_t LongDoubleToUInt64(long double v);

int64_t UInt64ToSInt64(uint64_t v);
uint64_t SInt64ToUInt64(int64_t v);

#ifdef __cplusplus
}
#endif

#endif

