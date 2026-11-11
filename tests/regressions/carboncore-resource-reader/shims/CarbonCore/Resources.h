// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <memory>
using SInt8 = int8_t;
using ResID = int16_t;
using ResType = uint32_t;
using ResAttributes = int16_t;
using ResFileAttributes = int16_t;
using OSErr = int16_t;
using Ptr = char*;
using Handle = Ptr*;
constexpr OSErr fnfErr = -43;
constexpr OSErr eofErr = -39;
constexpr OSErr noErr = 0;
constexpr SInt8 fsWrPerm = 2;
