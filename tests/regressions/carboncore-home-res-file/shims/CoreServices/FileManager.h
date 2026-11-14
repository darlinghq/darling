// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <CoreServices/MacTypes.h>
// Host-only path carrier, not the Darwin File Manager ABI.
struct FSRef { char path[4096]; };
using FSIORefNum = int16_t;
struct HFSUniStr255 { uint16_t length; UniChar unicode[255]; };
struct FSSpec { char unused[80]; };
using FSSpecPtr = FSSpec*;
using FSCatalogInfoBitmap = uint32_t;
struct FSCatalogInfo { struct { uint32_t mode; } permissions; };
constexpr FSCatalogInfoBitmap kFSCatInfoPermissions = 0x100;
constexpr SInt8 fsRdPerm = 1, fsWrPerm = 2;
extern "C" OSErr FSRefMakePath(const FSRef*, UInt8*, UInt32);
extern "C" OSErr FSPathMakeRef(const UInt8*, FSRef*, Boolean*);
extern "C" OSErr FSGetDataForkName(HFSUniStr255*);
extern "C" OSErr FSGetResourceForkName(HFSUniStr255*);
