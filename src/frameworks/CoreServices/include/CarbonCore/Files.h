/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

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


#ifndef _CS_FILES_H
#define _CS_FILES_H
#include <CoreServices/MacTypes.h>
#include <CoreServices/FileManager.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __LP64__
	typedef int FSIORefNum;
#else
	typedef SInt16 FSIORefNum;
#endif

enum {
	fsCurPerm = 0,
	fsRdPerm = 1,
	fsWrPerm = 2,
	fsRdWrPerm = 3,
	dsRdWrShPerm = 4,
};

typedef SInt16 FSVolumeRefNum;
typedef UInt32 FSVolumeInfoBitmap;

typedef struct FSVolumeInfo {
	UTCDateTime createDate;
	UTCDateTime modifyDate;
	UTCDateTime backupDate;
	UTCDateTime checkedDate;
	UInt32 fileCount;
	UInt32 folderCount;
	UInt64 totalBytes;
	UInt64 freeBytes;
	UInt32 blockSize;
	UInt32 totalBlocks;
	UInt32 freeBlocks;
	UInt32 nextAllocation;
	UInt32 rsrcClumpSize;
	UInt32 dataClumpSize;
	UInt32 nextCatalogID;
	UInt8 finderInfo[32];
	UInt16 flags;
	UInt16 filesystemID;
	UInt16 signature;
	UInt16 driveNumber;
	FSIORefNum driverRefNum;
} FSVolumeInfo;

OSErr FSGetDataForkName(HFSUniStr255* dataForkName);
OSErr FSGetResourceForkName(HFSUniStr255* rsrcForkName);

OSErr FSGetVolumeInfo(FSVolumeRefNum volume, ItemCount volumeIndex, FSVolumeRefNum *actualVolume, FSVolumeInfoBitmap whichInfo, FSVolumeInfo *info, HFSUniStr255 *volumeName, FSRef *rootDirectory);

#ifdef __cplusplus
}
#endif

#endif
