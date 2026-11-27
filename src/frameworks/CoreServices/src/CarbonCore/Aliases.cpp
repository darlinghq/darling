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

#include <CarbonCore/Aliases.h>
#include <cstdio>

#define STUB() std::printf("STUB %s\n", __PRETTY_FUNCTION__);

OSErr FSResolveAliasFile(FSRef *theRef, Boolean resolveAliasChains, Boolean *targetIsFolder, Boolean *wasAliased)
{
	STUB();

	return unimpErr;
}

// --- Alias records ---
//
// FSNewAlias remembers where an item is: its name, the volume it is on and its path, plus the
// creation dates and the directory flag when they are known. FSCopyAliasInfo reads that back
// without touching the file system. The record is a handle of the size GetAliasSize reports,
// starting with the user type (always 0 here) and the record size as big-endian numbers, followed
// by a private layout.

#include <CarbonCore/DateTimeUtils.h>
#include <CarbonCore/Files.h>
#include <CarbonCore/MacMemory.h>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <string>

namespace {

const uint16_t kAliasVersion = 1;
const uint8_t kAliasHasInfo = 1;
const uint8_t kAliasIsDirectory = 2;
const uint8_t kAliasBootVolume = 4;

// What follows the 6 byte header of the record. The strings are UTF-8, one after the other:
// the target's name, the volume's name and the path.
struct AliasBody {
	uint16_t version;
	uint8_t flags;
	uint8_t reserved;
	UTCDateTime volumeCreateDate;
	UTCDateTime targetCreateDate;
	uint16_t targetNameLength;
	uint16_t volumeNameLength;
	uint16_t pathLength;
} __attribute__((packed));

const size_t kAliasHeaderSize = 6;

bool uniStrFromUTF8(const std::string& text, HFSUniStr255* out)
{
	CFStringRef string = CFStringCreateWithBytes(NULL, (const UInt8*) text.data(), text.size(), kCFStringEncodingUTF8, false);
	if (string == NULL)
		return false;
	out->length = (uint16_t) std::min<CFIndex>(CFStringGetLength(string), 255);
	CFStringGetCharacters(string, CFRangeMake(0, out->length), out->unicode);
	CFRelease(string);
	return true;
}

std::string utf8FromUniStr(const HFSUniStr255& name)
{
	CFStringRef string = CFStringCreateWithCharacters(NULL, name.unicode, name.length);
	if (string == NULL)
		return std::string();
	char buffer[255 * 4 + 1];
	std::string result;
	if (CFStringGetCString(string, buffer, sizeof buffer, kCFStringEncodingUTF8))
		result = buffer;
	CFRelease(string);
	return result;
}

OSErr makeAlias(const FSRef* target, bool withInfo, AliasHandle* inAlias)
{
	if (inAlias == NULL)
		return paramErr;
	*inAlias = NULL;
	if (target == NULL)
		return paramErr;

	char path[PATH_MAX];
	OSStatus err = FSRefMakePath(target, (UInt8*) path, sizeof path);
	if (err != noErr)
		return err == fnfErr ? (OSErr) errFSBadFSRef : (OSErr) err;
	// The alias keeps the path with symbolic links resolved.
	char resolved[PATH_MAX];
	if (realpath(path, resolved) == NULL)
		return errFSBadFSRef;

	AliasBody body;
	memset(&body, 0, sizeof body);
	body.version = kAliasVersion;

	HFSUniStr255 targetName, volumeName;
	FSCatalogInfo catalog;
	FSVolumeInfo volume;
	FSRef resolvedRef;
	if (FSPathMakeRef((const UInt8*) resolved, &resolvedRef, NULL) != noErr)
		return errFSBadFSRef;
	err = FSGetCatalogInfo(&resolvedRef, kFSCatInfoCreateDate | kFSCatInfoNodeFlags | kFSCatInfoVolume, &catalog, &targetName, NULL, NULL);
	if (err != noErr)
		return err;
	FSVolumeRefNum volumeRef = catalog.volume;
	if (FSGetVolumeInfo(volumeRef, 0, NULL, withInfo ? (kFSVolInfoCreateDate | kFSVolInfoFlags) : kFSVolInfoNone, &volume, &volumeName, NULL) != noErr)
		volumeName.length = 0;
	if (withInfo) {
		body.flags |= kAliasHasInfo;
		body.volumeCreateDate = volume.createDate;
		body.targetCreateDate = catalog.createDate;
		if (catalog.nodeFlags & kFSNodeIsDirectoryMask)
			body.flags |= kAliasIsDirectory;
		if (volume.flags & kFSVolFlagDefaultVolumeMask)
			body.flags |= kAliasBootVolume;
	}

	std::string name = utf8FromUniStr(targetName);
	std::string volumeNameText = utf8FromUniStr(volumeName);
	std::string pathText = resolved;
	body.targetNameLength = (uint16_t) name.size();
	body.volumeNameLength = (uint16_t) volumeNameText.size();
	body.pathLength = (uint16_t) pathText.size();

	size_t size = kAliasHeaderSize + sizeof body + name.size() + volumeNameText.size() + pathText.size();
	if (size > 0xFFFF)
		return paramErr;
	Handle handle = NewHandle((long) size);
	if (handle == NULL)
		return memFullErr;
	uint8_t* bytes = (uint8_t*) *handle;
	memset(bytes, 0, 4);
	bytes[4] = (uint8_t) (size >> 8);
	bytes[5] = (uint8_t) size;
	uint8_t* cursor = bytes + kAliasHeaderSize;
	memcpy(cursor, &body, sizeof body);
	cursor += sizeof body;
	memcpy(cursor, name.data(), name.size());
	cursor += name.size();
	memcpy(cursor, volumeNameText.data(), volumeNameText.size());
	cursor += volumeNameText.size();
	memcpy(cursor, pathText.data(), pathText.size());
	*inAlias = (AliasHandle) handle;
	return noErr;
}

// The parts of an alias record, or false when the handle holds something else.
bool parseAlias(AliasHandle alias, AliasBody* body, std::string* name, std::string* volume, std::string* path)
{
	if (alias == NULL || *alias == NULL)
		return false;
	// The size in the header is what the record holds; the handle may be larger.
	long handleSize = GetAliasSize(alias);
	const uint8_t* bytes = (const uint8_t*) *alias;
	if (handleSize < (long) (kAliasHeaderSize + sizeof *body))
		return false;
	memcpy(body, bytes + kAliasHeaderSize, sizeof *body);
	size_t strings = (size_t) body->targetNameLength + body->volumeNameLength + body->pathLength;
	if (body->version != kAliasVersion || kAliasHeaderSize + sizeof *body + strings > (size_t) handleSize)
		return false;
	const char* text = (const char*) bytes + kAliasHeaderSize + sizeof *body;
	name->assign(text, body->targetNameLength);
	volume->assign(text + body->targetNameLength, body->volumeNameLength);
	path->assign(text + body->targetNameLength + body->volumeNameLength, body->pathLength);
	return true;
}

}

OSErr FSNewAlias(const FSRef* fromFile, const FSRef* target, AliasHandle* inAlias)
{
	// The alias holds the absolute path of the target, so the file it is made from has no effect.
	return makeAlias(target, true, inAlias);
}

OSErr FSNewAliasMinimal(const FSRef* target, AliasHandle* inAlias)
{
	return makeAlias(target, false, inAlias);
}

OSStatus FSCopyAliasInfo(AliasHandle inAlias, HFSUniStr255* targetName, HFSUniStr255* volumeName,
	CFStringRef* pathString, FSAliasInfoBitmap* whichInfo, FSAliasInfo* info)
{
	AliasBody body;
	std::string name, volume, path;
	if (!parseAlias(inAlias, &body, &name, &volume, &path))
		return paramErr;

	if (targetName && !uniStrFromUTF8(name, targetName))
		return paramErr;
	if (volumeName && !uniStrFromUTF8(volume, volumeName))
		return paramErr;
	if (pathString)
		*pathString = CFStringCreateWithBytes(NULL, (const UInt8*) path.data(), path.size(), kCFStringEncodingUTF8, false);

	FSAliasInfoBitmap available = kFSAliasInfoNone;
	if (body.flags & kAliasHasInfo)
		available = kFSAliasInfoVolumeCreateDate | kFSAliasInfoTargetCreateDate | kFSAliasInfoIsDirectory | kFSAliasInfoVolumeFlags;
	if (whichInfo)
		*whichInfo = available;
	if (info) {
		memset(info, 0, sizeof *info);
		if (available) {
			info->volumeCreateDate = body.volumeCreateDate;
			info->targetCreateDate = body.targetCreateDate;
			info->isDirectory = (body.flags & kAliasIsDirectory) != 0;
			info->volumeIsBootVolume = (body.flags & kAliasBootVolume) != 0;
		}
	}
	return noErr;
}

Size GetAliasSize(AliasHandle alias)
{
	if (alias == NULL || *alias == NULL)
		return 0;
	const uint8_t* bytes = (const uint8_t*) *alias;
	return (Size) (((unsigned) bytes[4] << 8) | bytes[5]);
}

#ifndef __LP64__
#include <CarbonCore/Aliases.h>

#ifdef __cplusplus
extern "C"
#endif
OSErr NewAlias(
    const FSSpec* fromFile,
    const FSSpec* target,
    AliasHandle* result)
{
    if (!result)
        return paramErr;

    *result=0;

    if (!target)
        return paramErr;

    FSRef targetRef;
    OSErr err=FSpMakeFSRef(target,&targetRef);

    if (err!=noErr)
        return err;

    FSRef fromRef;
    const FSRef* origin=0;

    if (fromFile) {
        err=FSpMakeFSRef(fromFile,&fromRef);

        if (err!=noErr)
            return err;

        origin=&fromRef;
    }

    return FSNewAlias(origin,&targetRef,result);
}

#endif
