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

#ifndef COREENDIAN_H
#define COREENDIAN_H
#include <MacTypes.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

UInt16 Endian16_Swap(UInt16 value);
UInt32 Endian32_Swap(UInt32 value);
UInt64 Endian64_Swap(UInt64 value);

SInt16 EndianS16_BtoL(SInt16 value);
SInt16 EndianS16_BtoN(SInt16 value);
SInt16 EndianS16_LtoB(SInt16 value);
SInt16 EndianS16_LtoN(SInt16 value);
SInt16 EndianS16_NtoB(SInt16 value);
SInt16 EndianS16_NtoL(SInt16 value);

SInt32 EndianS32_BtoL(SInt32 value);
SInt32 EndianS32_BtoN(SInt32 value);
SInt32 EndianS32_LtoB(SInt32 value);
SInt32 EndianS32_LtoN(SInt32 value);
SInt32 EndianS32_NtoB(SInt32 value);
SInt32 EndianS32_NtoL(SInt32 value);

SInt64 EndianS64_BtoL(SInt64 value);
SInt64 EndianS64_BtoN(SInt64 value);
SInt64 EndianS64_LtoB(SInt64 value);
SInt64 EndianS64_LtoN(SInt64 value);
SInt64 EndianS64_NtoB(SInt64 value);
SInt64 EndianS64_NtoL(SInt64 value);

UInt16 EndianU16_BtoL(UInt16 value);
UInt16 EndianU16_BtoN(UInt16 value);
UInt16 EndianU16_LtoB(UInt16 value);
UInt16 EndianU16_LtoN(UInt16 value);
UInt16 EndianU16_NtoB(UInt16 value);
UInt16 EndianU16_NtoL(UInt16 value);

UInt32 EndianU32_BtoL(UInt32 value);
UInt32 EndianU32_BtoN(UInt32 value);
UInt32 EndianU32_LtoB(UInt32 value);
UInt32 EndianU32_LtoN(UInt32 value);
UInt32 EndianU32_NtoB(UInt32 value);
UInt32 EndianU32_NtoL(UInt32 value);

UInt64 EndianU64_BtoL(UInt64 value);
UInt64 EndianU64_BtoN(UInt64 value);
UInt64 EndianU64_LtoB(UInt64 value);
UInt64 EndianU64_LtoN(UInt64 value);
UInt64 EndianU64_NtoB(UInt64 value);
UInt64 EndianU64_NtoL(UInt64 value);

typedef OSStatus (*CEFlipper)(uint32_t dataDomain, uint32_t dataType, int16_t id, void* data, unsigned long length, Boolean isNative, void* opaque);

OSStatus CoreEndianFlipData(uint32_t dataDomain, uint32_t dataType, int16_t id, void* data, unsigned long length, Boolean isNative);
OSStatus CoreEndianInstallFlipper(uint32_t dataDomain, uint32_t dataType, CEFlipper flipper, void* opaque);
OSStatus CoreEndianGetFlipper(uint32_t dataDomain, uint32_t dataType, CEFlipper* flipper, void** opaque);

#ifdef __cplusplus
}
#endif

#endif
