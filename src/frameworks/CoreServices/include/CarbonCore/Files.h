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
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	fsCurPerm = 0,
	fsRdPerm = 1,
	fsWrPerm = 2,
	fsRdWrPerm = 3,
	fsRdWrShPerm = 4,
	dsRdWrShPerm = 4,
};

typedef SInt16 FSVolumeRefNum;
typedef UInt32 FSVolumeInfoBitmap;

enum {
	kFSInvalidVolumeRefNum = 0
};

enum {
	fsRtParID = 1,
	fsRtDirID = 2
};

// Positioning modes of the fork I/O calls (ioPosMode / positionMode).
enum {
	fsAtMark = 0,
	fsFromStart = 1,
	fsFromLEOF = 2,
	fsFromMark = 3
};

// Flags that can be or-ed into a positioning mode.
enum {
	kFSAllowConcurrentAsyncIOBit = 3,
	kFSAllowConcurrentAsyncIOMask = 0x0008,
	kFSPleaseCacheBit = 4,
	kFSPleaseCacheMask = 0x0010,
	kFSNoCacheBit = 5,
	kFSNoCacheMask = 0x0020,
	kFSForceReadBit = 6,
	kFSForceReadMask = 0x0040,
	kFSNewLineBit = 7,
	kFSNewLineMask = 0x0080,
	kFSNewLineCharMask = 0xFF00
};

typedef struct OpaqueFSIterator* FSIterator;

enum {
	kFSIterateFlat = 0,
	kFSIterateSubtree = 1,
	kFSIterateDelete = 2,
	kFSIterateReserved = (int) 0xFFFFFFFC
};
typedef OptionBits FSIteratorFlags;

// Which FSVolumeInfo fields FSGetVolumeInfo fills (FSVolumeInfoBitmap).
enum {
	kFSVolInfoNone = 0x0000,
	kFSVolInfoCreateDate = 0x0001,
	kFSVolInfoModDate = 0x0002,
	kFSVolInfoBackupDate = 0x0004,
	kFSVolInfoCheckedDate = 0x0008,
	kFSVolInfoFileCount = 0x0010,
	kFSVolInfoDirCount = 0x0020,
	kFSVolInfoSizes = 0x0040,
	kFSVolInfoBlocks = 0x0080,
	kFSVolInfoNextAlloc = 0x0100,
	kFSVolInfoRsrcClump = 0x0200,
	kFSVolInfoDataClump = 0x0400,
	kFSVolInfoNextID = 0x0800,
	kFSVolInfoFinderInfo = 0x1000,
	kFSVolInfoFlags = 0x2000,
	kFSVolInfoFSInfo = 0x4000,
	kFSVolInfoDriveInfo = 0x8000,
	kFSVolInfoGettableInfo = 0xFFFF,
	kFSVolInfoSettableInfo = 0x3004
};

// FSVolumeInfo.flags bits.
enum {
	kFSVolFlagDefaultVolumeMask = 0x0020,
	kFSVolFlagFilesOpenMask = 0x0040,
	kFSVolFlagHardwareLockedMask = 0x0080,
	kFSVolFlagJournalingActiveMask = 0x4000
};

enum {
	kFSEjectVolumeForceEject = (1 << 0)
};

#pragma pack(push, 2)
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
#pragma pack(pop)

OSErr FSGetDataForkName(HFSUniStr255* dataForkName);
OSErr FSGetResourceForkName(HFSUniStr255* rsrcForkName);

OSErr FSOpenFork(const FSRef* ref, UniCharCount forkNameLength, const UniChar* forkName, SInt8 permissions, FSIORefNum* forkRefNum);
OSErr FSGetForkSize(FSIORefNum forkRefNum, SInt64* forkSize);
OSErr FSCloseFork(FSIORefNum forkRefNum);

OSErr FSReadFork(FSIORefNum forkRefNum, UInt16 positionMode, SInt64 positionOffset, ByteCount requestCount, void* buffer, ByteCount* actualCount);
OSErr FSWriteFork(FSIORefNum forkRefNum, UInt16 positionMode, SInt64 positionOffset, ByteCount requestCount, const void* buffer, ByteCount* actualCount);
OSErr FSSetForkSize(FSIORefNum forkRefNum, UInt16 positionMode, SInt64 positionOffset);
#if !__LP64__
OSErr SetFPos(FSIORefNum refNum, SInt16 posMode, SInt32 posOff);
#endif

OSErr FSGetForkPosition(FSIORefNum forkRefNum, SInt64* position);
OSErr FSSetForkPosition(FSIORefNum forkRefNum, UInt16 positionMode, SInt64 positionOffset);

OSErr FSOpenIterator(const FSRef* container, FSIteratorFlags iteratorFlags, FSIterator* iterator);
OSErr FSCloseIterator(FSIterator iterator);
OSErr FSGetCatalogInfoBulk(FSIterator iterator, ItemCount maximumObjects, ItemCount* actualObjects, Boolean* containerChanged, FSCatalogInfoBitmap whichInfo, FSCatalogInfo* catalogInfos, FSRef* refs, FSSpecPtr specs, HFSUniStr255* names);

OSStatus FSEjectVolumeSync(FSVolumeRefNum vRefNum, OptionBits flags, pid_t* dissenter);

OSErr FSGetVolumeInfo(FSVolumeRefNum volume, ItemCount volumeIndex, FSVolumeRefNum *actualVolume, FSVolumeInfoBitmap whichInfo, FSVolumeInfo *info, HFSUniStr255 *volumeName, FSRef *rootDirectory);

// Version 1 to 5 layouts of the buffer PBHGetVolParmsSync fills; the version is in vMVersion.
#pragma pack(push, 2)
struct GetVolParmsInfoBuffer {
	SInt16 vMVersion;
	SInt32 vMAttrib;
	Handle vMLocalHand;
	SInt32 vMServerAdr;
	SInt32 vMVolumeGrade;
	SInt16 vMForeignPrivID;
	SInt32 vMExtendedAttributes;
	void* vMDeviceID;
	UniCharCount vMMaxNameLength;
};
typedef struct GetVolParmsInfoBuffer GetVolParmsInfoBuffer;
#pragma pack(pop)

// Fills as much of the buffer as bufferSize allows with the volume's parameters.
OSStatus FSGetVolumeParms(FSVolumeRefNum volume, GetVolParmsInfoBuffer* buffer, ByteCount bufferSize);

// Volume attribute bits of GetVolParmsInfoBuffer.vMAttrib.
enum {
	bLimitFCBs = 31,
	bLocalWList = 30,
	bNoMiniFndr = 29,
	bNoVNEdit = 28,
	bNoLclSync = 27,
	bTrshOffLine = 26,
	bNoSwitchTo = 25,
	bNoDeskItems = 20,
	bNoBootBlks = 19,
	bAccessCntl = 18,
	bNoSysDir = 17,
	bHasExtFSVol = 16,
	bHasOpenDeny = 15,
	bHasCopyFile = 14,
	bHasMoveRename = 13,
	bHasDesktopMgr = 12,
	bHasShortName = 11,
	bHasFolderLock = 10,
	bHasPersonalAccessPrivileges = 9,
	bHasUserGroupList = 8,
	bHasCatSearch = 7,
	bHasFileIDs = 6,
	bHasBTreeMgr = 5,
	bHasBlankAccessPrivileges = 4,
	bSupportsAsyncRequests = 3,
	bSupportsTrashVolumeCache = 2
};

// The I/O flavour of the legacy File Manager parameter block; PBHGetVolParmsSync reads the
// buffer fields and writes ioActCount.
#pragma pack(push, 2)
struct HIOParam {
	QElemPtr qLink;
	SInt16 qType;
	SInt16 ioTrap;
	Ptr ioCmdAddr;
	IOCompletionUPP ioCompletion;
	volatile OSErr ioResult;
	StringPtr ioNamePtr;
	FSVolumeRefNum ioVRefNum;
	FSIORefNum ioRefNum;
	SInt8 ioVersNum;
	SInt8 ioPermssn;
	Ptr ioMisc;
	Ptr ioBuffer;
	SInt32 ioReqCount;
	SInt32 ioActCount;
	SInt16 ioPosMode;
	SInt32 ioPosOffset;
};
#pragma pack(pop)
typedef struct HIOParam HIOParam;
typedef HIOParam* HIOParamPtr;

// Only the I/O member is declared; the other members of the original union are not used by the
// calls Darling provides, so the padding keeps the size at least as large as theirs.
#pragma pack(push, 2)
union HParamBlockRec {
	HIOParam ioParam;
	char reserved[256];
};
typedef union HParamBlockRec HParamBlockRec;
#pragma pack(pop)
typedef HParamBlockRec* HParmBlkPtr;

OSErr PBHGetVolParmsSync(HParmBlkPtr paramBlock);

enum {
	kFSFileOperationDefaultOptions = 0,
	kFSFileOperationOverwrite = 1,
	kFSFileOperationSkipSourcePermissionCheck = 2,
	kFSFileOperationDoNotMoveAcrossVolumes = 4,
	kFSFileOperationSkipPreflight = 8
};

OSStatus FSCopyObjectSync(const FSRef *source, const FSRef *destinationDir, CFStringRef copyName, FSRef *newRef, OptionBits whichInfo);
OSStatus FSMoveObjectSync(const FSRef *source, const FSRef *destinationDir, CFStringRef moveName, FSRef *newRef, OptionBits whichInfo);
OSStatus FSMoveObjectToTrashSync(const FSRef *source, FSRef *target, OptionBits options);

OSErr FSCreateFileUnicode(const FSRef* parentRef, UniCharCount nameLength, const UniChar* name,
	FSCatalogInfoBitmap whichInfo, const FSCatalogInfo* catalogInfo, FSRef* newRef, FSSpecPtr newSpec);
OSStatus FSMakeFSRefUnicode(const FSRef* parentRef, UniCharCount nameLength, const UniChar* name,
	TextEncoding textEncodingHint, FSRef* newRef);
CFStringRef FSCreateStringFromHFSUniStr(CFAllocatorRef alloc, const HFSUniStr255* uniStr);
OSStatus FSGetHFSUniStrFromString(CFStringRef theString, HFSUniStr255* theOutString);

#if !__LP64__
// The Finder information of a file
#pragma pack(push, 2)
struct FInfo {
	OSType fdType;
	OSType fdCreator;
	UInt16 fdFlags;
	Point fdLocation;
	SInt16 fdFldr;
};
#pragma pack(pop)
typedef struct FInfo FInfo;

// The FSSpec calls of the 32-bit File Manager
OSErr FSMakeFSSpec(FSVolumeRefNum vRefNum, SInt32 dirID, ConstStr255Param fileName, FSSpec* spec);
OSStatus FSpMakeFSRef(const FSSpec* source, struct FSRef* newRef);
OSErr FSpCreate(const FSSpec* spec, OSType creator, OSType fileType, ScriptCode scriptTag);
OSErr FSpDelete(const FSSpec* spec);
OSErr FSpGetFInfo(const FSSpec* spec, FInfo* fndrInfo);
OSErr FSpSetFInfo(const FSSpec* spec, const FInfo* fndrInfo);
OSErr FSpOpenDF(const FSSpec* spec, SInt8 permission, FSIORefNum* refNum);
OSErr FSpOpenRF(const FSSpec* spec, SInt8 permission, FSIORefNum* refNum);
OSErr FSRead(FSIORefNum refNum, long* count, void* buffPtr);
OSErr FSWrite(FSIORefNum refNum, long* count, const void* buffPtr);
OSErr GetEOF(FSIORefNum refNum, long* logEOF);
OSErr SetEOF(FSIORefNum refNum, long logEOF);
OSErr GetFPos(FSIORefNum refNum, long* filePos);
OSErr FSClose(FSIORefNum refNum);
#endif


#ifdef __cplusplus
}
#endif

#endif
