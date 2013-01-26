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

#include "CoreEndian.h"
#include <endian.h>
#include <stdint.h>
#include <map>
#include "MacErrors.h"
#include "util/mutex.h"

static std::map<
	std::pair<uint32_t,uint32_t>,
	std::pair<CEFlipper,void*>
	> g_flippers;
static Darling::Mutex g_flippersMutex;

template <typename T> T bswap(T value);

template <> uint16_t bswap(uint16_t value)
{
	return __builtin_bswap16(value);
}

template <> uint32_t bswap(uint32_t value)
{
	return __builtin_bswap32(value);
}

template <> uint64_t bswap(uint64_t value)
{
	return __builtin_bswap64(value);
}

template <typename T> T LtoN(T value)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	return value;
#else
	return bswap(value);
#endif
}

template <typename T> T BtoN(T value)
{
#if __BYTE_ORDER == __BIG_ENDIAN
	return value;
#else
	return bswap(value);
#endif
}

template <typename T> T NtoL(T value)
{
	return LtoN(value);
}

template <typename T> T NtoB(T value)
{
	return BtoN(value);
}

UInt16 Endian16_Swap(UInt16 value)
{
	return bswap(value);
}

UInt32 Endian32_Swap(UInt32 value)
{
	return bswap(value);
}

UInt64 Endian64_Swap(UInt64 value)
{
	return bswap(value);
}

SInt16 EndianS16_BtoL(SInt16 value)
{
	return bswap(value);
}

SInt16 EndianS16_BtoN(SInt16 value)
{
	return BtoN(value);
}

SInt16 EndianS16_LtoB(SInt16 value)
{
	return bswap(value);
}

SInt16 EndianS16_LtoN(SInt16 value)
{
	return LtoN(value);
}

SInt16 EndianS16_NtoB(SInt16 value)
{
	return NtoB(value);
}

SInt16 EndianS16_NtoL(SInt16 value)
{
	return NtoL(value);
}

SInt32 EndianS32_BtoL(SInt32 value)
{
	return bswap(value);
}

SInt32 EndianS32_BtoN(SInt32 value)
{
	return BtoN(value);
}

SInt32 EndianS32_LtoB(SInt32 value)
{
	return bswap(value);
}

SInt32 EndianS32_LtoN(SInt32 value)
{
	return LtoN(value);
}

SInt32 EndianS32_NtoB(SInt32 value)
{
	return NtoB(value);
}

SInt32 EndianS32_NtoL(SInt32 value)
{
	return NtoL(value);
}


SInt64 EndianS64_BtoL(SInt64 value)
{
	return bswap(value);
}

SInt64 EndianS64_BtoN(SInt64 value)
{
	return BtoN(value);
}

SInt64 EndianS64_LtoB(SInt64 value)
{
	return bswap(value);
}

SInt64 EndianS64_LtoN(SInt64 value)
{
	return LtoN(value);
}

SInt64 EndianS64_NtoB(SInt64 value)
{
	return NtoB(value);
}

SInt64 EndianS64_NtoL(SInt64 value)
{
	return NtoL(value);
}

UInt16 EndianU16_BtoL(UInt16 value)
{
	return bswap(value);
}

UInt16 EndianU16_BtoN(UInt16 value)
{
	return BtoN(value);
}

UInt16 EndianU16_LtoB(UInt16 value)
{
	return bswap(value);
}

UInt16 EndianU16_LtoN(UInt16 value)
{
	return LtoN(value);
}

UInt16 EndianU16_NtoB(UInt16 value)
{
	return NtoB(value);
}

UInt16 EndianU16_NtoL(UInt16 value)
{
	return NtoL(value);
}

UInt32 EndianU32_BtoL(UInt32 value)
{
	return bswap(value);
}

UInt32 EndianU32_BtoN(UInt32 value)
{
	return BtoN(value);
}

UInt32 EndianU32_LtoB(UInt32 value)
{
	return bswap(value);
}

UInt32 EndianU32_LtoN(UInt32 value)
{
	return LtoN(value);
}

UInt32 EndianU32_NtoB(UInt32 value)
{
	return NtoB(value);
}

UInt32 EndianU32_NtoL(UInt32 value)
{
	return NtoL(value);
}

UInt64 EndianU64_BtoL(UInt64 value)
{
	return bswap(value);
}

UInt64 EndianU64_BtoN(UInt64 value)
{
	return BtoN(value);
}

UInt64 EndianU64_LtoB(UInt64 value)
{
	return bswap(value);
}

UInt64 EndianU64_LtoN(UInt64 value)
{
	return LtoN(value);
}

UInt64 EndianU64_NtoB(UInt64 value)
{
	return NtoB(value);
}

UInt64 EndianU64_NtoL(UInt64 value)
{
	return NtoL(value);
}

OSStatus CoreEndianFlipData(uint32_t dataDomain, uint32_t dataType, int16_t id, void* data, unsigned long length, Boolean isNative)
{
	Darling::MutexLock l(g_flippersMutex);
	auto it = g_flippers.find(std::make_pair(dataDomain, dataType));
	
	if (it == g_flippers.end())
		return handlerNotFoundErr;
	
	return it->second.first(dataDomain, dataType, id, data, length, isNative, it->second.second);
}

OSStatus CoreEndianInstallFlipper(uint32_t dataDomain, uint32_t dataType, CEFlipper flipper, void* opaque)
{
	Darling::MutexLock l(g_flippersMutex);
	g_flippers[std::make_pair(dataDomain, dataType)] = std::make_pair(flipper, opaque);
	return noErr;
}

OSStatus CoreEndianGetFlipper(uint32_t dataDomain, uint32_t dataType, CEFlipper* flipper, void** opaque)
{
	Darling::MutexLock l(g_flippersMutex);
	auto it = g_flippers.find(std::make_pair(dataDomain, dataType));
	
	*opaque = nullptr;
	*flipper = nullptr;
	
	if (it == g_flippers.end())
		return paramErr;
	
	*flipper = it->second.first;
	*opaque = it->second.second;
	
	return noErr;
}
