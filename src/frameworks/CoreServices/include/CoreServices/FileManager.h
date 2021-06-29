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
#include "MacTypes.h"
#include <CoreFoundation/CFURL.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FSRef_MAX_DEPTH (80 / sizeof(ino_t))

typedef struct FSRef
{
	uint8_t hidden[80];
} FSRef;
typedef struct FSRef* FSRefPtr;
typedef struct FSRef FSRef;

struct HFSUniStr255
{
	uint16_t length;
	uint16_t unicode[255];
};
typedef struct HFSUniStr255 HFSUniStr255;

struct FSSpec {
	char hidden[70];
};
typedef struct FSSpec FSSpec;

typedef struct FSSpec* FSSpecPtr;

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
	struct UTCDateTime createDate;
	struct UTCDateTime contentModDate;
	struct UTCDateTime attributeModDate;
	struct UTCDateTime accessDate;
	struct UTCDateTime backupDate;

	union
	{
		uint32_t permissions[4];
		struct FSPermissionInfo fsPermissionInfo;
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
typedef struct FSCatalogInfo FSCatalogInfo;

typedef void* IOCompletionUPP;
typedef void* QElemPtr;
typedef uint16_t FSAllocationFlags;
typedef uint32_t FSCatalogInfoBitmap;
typedef unsigned long UniCharCount;
typedef UInt32 TextEncoding;

struct FSRefParam
{
	QElemPtr qLink;
	short qType;
	short ioTrap;
	Ptr ioCmdAddr;
	IOCompletionUPP ioCompletion;
	volatile OSErr ioResult;
	const Str255* ioNamePtr;
	short ioVRefNum;
	SInt16 reserved1;
	UInt8 reserved2;
	UInt8 reserved3;
	const FSRefPtr ref;
	FSCatalogInfoBitmap whichInfo;
	struct FSCatalogInfo* catInfo;
	UniCharCount nameLength;
	const UniChar* name;
	long ioDirID;
	FSSpecPtr spec;
	FSRefPtr parentRef;
	FSRefPtr newRef;
	TextEncoding textEncodingHint;
	struct HFSUniStr255* outName;
};

struct CatPositionRec
{
	long initialize;
	short priv[6];
};

struct FSForkIOParam
{
	QElemPtr qLink;
	short qType;
	short ioTrap;
	Ptr ioCmdAddr;
	IOCompletionUPP ioCompletion;
	volatile OSErr ioResult;
	void * reserved1;
	SInt16 reserved2;
	SInt16 forkRefNum;
	UInt8 reserved3;
	SInt8 permissions;
	const FSRefPtr ref;
	Ptr buffer;
	UInt32 requestCount;
	UInt32 actualCount;
	UInt16 positionMode;
	SInt64 positionOffset;
	FSAllocationFlags allocationFlags;
	UInt64 allocationAmount;
	UniCharCount forkNameLength;
	const UniChar * forkName;
	struct CatPositionRec forkIterator;
	struct HFSUniStr255* outForkName;
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

enum
{
	kSystemFolderType = 'macs',
	kDesktopFolderType = 'desk',
	kSystemDesktopFolderType = 'sdsk',
	kTrashFolderType = 'trsh',
	kSystemTrashFolderType = 'strs',
	kWhereToEmptyTrashFolderType = 'empt',
	kPrintMonitorDocsFolderType = 'prnt',
	kStartupFolderType = 'strt',
	kShutdownFolderType = 'shdf',
	kAppleMenuFolderType = 'amnu',
	kControlPanelFolderType = 'ctrl',
	kSystemControlPanelFolderType = 'sctl',
	kExtensionFolderType = 'extn',
	kFontsFolderType = 'font',
	kPreferencesFolderType = 'pref',
	kSystemPreferencesFolderType = 'sprf',
	kTemporaryFolderType = 'temp'
};

OSStatus FSPathMakeRef(const uint8_t* path, struct FSRef* fsref, Boolean* isDirectory);
OSStatus FSPathMakeRefWithOptions(const uint8_t* path, long options, struct FSRef* fsref, Boolean* isDirectory);
OSStatus FSRefMakePath(const struct FSRef* fsref, uint8_t* path, uint32_t maxSize);
Boolean CFURLGetFSRef(CFURLRef urlref, struct FSRef* fsref); // in CF
CFURLRef CFURLCreateFromFSRef(CFAllocatorRef alloc, struct FSRef* location); // --> in CF
OSStatus FSFindFolder(long vRefNum, OSType folderType, Boolean createFolder, struct FSRef* location);
OSStatus FSDeleteObject(const FSRef* fsref);

OSStatus FSGetCatalogInfo(const FSRefPtr ref, uint32_t infoBits, struct FSCatalogInfo* infoOut, struct HFSUniStr255* nameOut, FSSpecPtr fsspec, FSRefPtr parentDir);

OSErr PBCreateDirectoryUnicodeSync(struct FSRefParam* paramBlock);
OSErr PBCreateFileUnicodeSync(struct FSRefParam* paramBlock);
OSErr PBGetCatalogInfoSync(struct FSRefParam *paramBlock);
OSErr PBMakeFSRefUnicodeSync(struct FSRefParam *paramBlock);
OSErr PBOpenForkSync(struct FSForkIOParam *paramBlock);
OSErr PBReadForkSync(struct FSForkIOParam *paramBlock);
OSErr PBWriteForkSync(struct FSForkIOParam *paramBlock);
OSErr PBIterateForksSync(struct FSForkIOParam *paramBlock);
OSErr PBCloseForkSync(struct FSForkIOParam *paramBlock);

#ifdef __cplusplus
}
#endif

#endif

