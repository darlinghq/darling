/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef _CC_ALIASES_H
#define _CC_ALIASES_H

#include <CoreServices/FileManager.h>
#include <CoreFoundation/CFString.h>

#ifdef __cplusplus
extern "C" {
#endif

OSErr FSResolveAliasFile(FSRef *theRef, Boolean resolveAliasChains, Boolean *targetIsFolder, Boolean *wasAliased);

// The Alias Manager's record, as seen by applications: a handle whose first bytes are the user type
// (4 bytes) and the size of the record in bytes (big-endian, 2 bytes); the rest is private.
struct AliasRecord {
	UInt8 hidden[6];
};
typedef struct AliasRecord AliasRecord;
typedef AliasRecord *AliasPtr;
typedef AliasPtr *AliasHandle;

typedef UInt32 FSAliasInfoBitmap;
enum {
	kFSAliasInfoNone = 0x00000000,
	kFSAliasInfoVolumeCreateDate = 0x00000001,
	kFSAliasInfoTargetCreateDate = 0x00000002,
	kFSAliasInfoFinderInfo = 0x00000004,
	kFSAliasInfoIsDirectory = 0x00000008,
	kFSAliasInfoIDs = 0x00000010,
	kFSAliasInfoFSInfo = 0x00000020,
	kFSAliasInfoVolumeFlags = 0x00000040
};

struct FSAliasInfo {
	UTCDateTime volumeCreateDate;
	UTCDateTime targetCreateDate;
	OSType fileType;
	OSType fileCreator;
	UInt32 parentDirID;
	UInt32 nodeID;
	UInt16 filesystemID;
	UInt16 signature;
	Boolean volumeIsBootVolume;
	Boolean volumeIsAutomounted;
	Boolean volumeIsEjectable;
	Boolean volumeHasPersistentFileIDs;
	Boolean isDirectory;
};
typedef struct FSAliasInfo FSAliasInfo;
typedef FSAliasInfo *FSAliasInfoPtr;

OSErr FSNewAlias(const FSRef *fromFile, const FSRef *target, AliasHandle *inAlias);
OSErr FSNewAliasMinimal(const FSRef *target, AliasHandle *inAlias);
OSStatus FSCopyAliasInfo(AliasHandle inAlias, HFSUniStr255 *targetName, HFSUniStr255 *volumeName,
	CFStringRef *pathString, FSAliasInfoBitmap *whichInfo, FSAliasInfo *info);
Size GetAliasSize(AliasHandle alias);

#ifdef __cplusplus
}
#endif

#endif
