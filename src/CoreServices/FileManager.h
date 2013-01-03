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


#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <stdint.h>
#include <dirent.h>
#include "MacErrors.h"
#include "DateTimeUtils.h"
#include <CoreFoundation/CFURL.h>

extern "C" {

constexpr size_t FSRef_MAX_DEPTH = 80 / sizeof(ino_t);

struct FSRef
{
	union
	{
		uint8_t hidden[80];

		// Inode numbers leading to the file in the directory structure
		//
		// EXAMPLES:
		// All zeroes: /
		// sequence 1,2,0: [dir with inode 1]/[dir or file with inode 2]
		ino_t inodes[FSRef_MAX_DEPTH];
	};
};
typedef FSRef* FSRefPtr;

struct HFSUniStr255
{
	uint16_t length;
	uint16_t unicode[255];
};
struct FSSpec;

typedef FSSpec* FSSpecPtr;

struct FSPermissionInfo
{
	uint32_t userID, groupID; // uid, gid
	uint8_t reserved1;
	uint8_t userAccess; // mode for the current user
	uint16_t mode; // mode
	uint32_t reserved2;
};

struct FSCatalogInfo
{
	uint16_t nodeFlags;
	int16_t volume;
	uint32_t parentDirID;
	uint32_t nodeID;
	uint8_t sharingFlags;
	uint8_t userPrivileges;
	uint8_t reserved1;
	uint8_t reserved2;
	UTCDateTime createDate;
	UTCDateTime contentModDate;
	UTCDateTime attributeModDate;
	UTCDateTime accessDate;
	UTCDateTime backupDate;

	union
	{
		uint32_t permissions[4];
		FSPermissionInfo fsPermissionInfo;
	};

	uint8_t finderInfo[16];
	uint8_t extFinderInfo[16];
	uint64_t dataLogicalSize;
	uint64_t dataPhysicalSize;
	uint64_t rsrcLogicalSize;
	uint64_t rsrcPhysicalSize;
	uint32_t valence; // file count within a directory
	uint32_t textEncodingHint;
};

enum
{
	kFSPathMakeRefDefaultOptions = 0,
	kFSPathMakeRefDoNotFollowLeafSymlink = 1
};

enum
{
	kFSCatInfoNone = 0x0,
	kFSCatInfoTextEncoding = 0x1,
	kFSCatInfoNodeFlags = 0x2,
	kFSCatInfoVolume = 0x4,
	kFSCatInfoParentDirID = 0x8,
	kFSCatInfoNodeID = 0x10,
	kFSCatInfoCreateDate = 0x20,
	kFSCatInfoContentMod = 0x40,
	kFSCatInfoAttrMod = 0x80,
	kFSCatInfoAccessDate = 0x100,
	kFSCatInfoBackupDate = 0x200,
	kFSCatInfoPermissions = 0x400,
	kFSCatInfoFinderInfo = 0x800,
	kFSCatInfoFinderXInfo = 0x1000,
	kFSCatInfoValence = 0x2000,
	kFSCatInfoDataSizes = 0x4000,
	kFSCatInfoRsrcSizes = 0x8000,
	kFSCatInfoSharingFlags = 0x10000,
	kFSCatInfoUserPrivs = 0x20000,
	kFSCatInfoUserAccess = 0x80000,
	kFSCatInfoSetOwnership = 0x100000
};

OSStatus FSPathMakeRef(const uint8_t* path, FSRef* fsref, Boolean* isDirectory);
OSStatus FSPathMakeRefWithOptions(const uint8_t* path, long options, FSRef* fsref, Boolean* isDirectory);
OSStatus FSRefMakePath(const FSRef* fsref, uint8_t* path, uint32_t maxSize);
Boolean CFURLGetFSRef(CFURLRef urlref, FSRef* fsref);

OSStatus FSGetCatalogInfo(const FSRef* ref, uint32_t infoBits, FSCatalogInfo* infoOut, HFSUniStr255* nameOut, FSSpecPtr fsspec, FSRef* parentDir);

}

#endif

