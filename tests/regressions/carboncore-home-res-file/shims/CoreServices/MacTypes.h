// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <cstddef>
using SInt8 = int8_t;
using SInt16 = int16_t;
using UInt8 = uint8_t;
using UInt32 = uint32_t;
using UInt64 = uint64_t;
using UniChar = uint16_t;
using UniCharCount = uint32_t;
using Boolean = uint8_t;
using OSErr = int16_t;
using Size = long;
using ResType = uint32_t;
using Ptr = char*;
using Handle = Ptr*;
using Str255 = unsigned char[256];
using StringPtr = unsigned char*;
using ConstStr255Param = const unsigned char*;
constexpr OSErr noErr = 0, unimpErr = -4, fnfErr = -43, eofErr = -39;
constexpr OSErr paramErr = -50, dirNFErr = -120, bdNamErr = -37;
constexpr OSErr errFSBadForkName = -1409, dupFNErr = -48, permErr = -54, ioErr = -36;
constexpr OSErr resNotFound = -192, resFNotFound = -193, inputOutOfBounds = -190;
constexpr OSErr rmvResFailed = -196, addResFailed = -194, writingPastEnd = -189;
