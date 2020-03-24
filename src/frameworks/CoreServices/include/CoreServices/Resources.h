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


#ifndef _CS_RESOURCES_H
#define _CS_RESOURCES_H
#include <CoreServices/MacTypes.h>
#include <CoreServices/Files.h>
#include <CoreServices/FileManager.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	kResFileNotOpened = -1,
	kSystemResFile = 0,
};

typedef SInt16 ResID;
typedef SInt16 ResAttributes;
typedef SInt16 ResFileAttributes;
typedef SInt16 ResourceCount;
typedef SInt16 ResourceIndex;
typedef FSIORefNum ResFileRefNum;

void CloseResFile(ResFileRefNum refNum);
OSErr ResError(void);
ResFileRefNum CurResFile(void);
ResFileRefNum HomeResFile(Handle resource);
void UseResFile(ResFileRefNum ref);

// NOTE: What's the difference between CountTypes() and Count1Types()?
// CountTypes() applies to *all* open resource files.
// Count1Types() applies to CurResFile() only.

ResourceCount CountTypes(void);
ResourceCount Count1Types(void);

void GetIndType(ResType* type, ResourceIndex index);
void Get1IndType(ResType* type, ResourceIndex index);

void SetResLoad(Boolean load);

ResourceCount CountResources(ResType type);
ResourceCount Count1Resources(ResType type);

Handle GetIndResource(ResType type, ResourceIndex index);
Handle Get1IndResource(ResType type, ResourceIndex index);

Handle GetResource(ResType type, ResID resId);
Handle Get1Resource(ResType type, ResID resId);

Handle GetNamedResource(ResType type, ConstStr255Param name);
Handle Get1NamedResource(ResType type, ConstStr255Param name);

void LoadResource(Handle res);
void ReleaseResource(Handle res);
void DetachResource(Handle res);

ResID UniqueID(ResType type);
ResID Unique1ID(ResType type);

ResAttributes GetResAttrs(Handle res);
void GetResInfo(Handle res, ResID* resId, ResType* type, Str255 name);
void SetResInfo(Handle res, ResID theID, ConstStr255Param name);

void AddResource(Handle data, ResType type, ResID resId, ConstStr255Param name);
long GetResourceSizeOnDisk(Handle res);
long GetMaxResourceSize(Handle res);

void SetResAttrs(Handle res, ResAttributes attrs);
void ChangedResource(Handle res);
void RemoveResource(Handle res);
void UpdateResFile(ResFileRefNum refNum);
void WriteResource(Handle res);

void SetResPurge(Boolean install);

ResFileAttributes GetResFileAttrs(ResFileRefNum refNum);
void SetResFileAttrs(ResFileRefNum refNum, ResFileAttributes attrs);

void ReadPartialResource(Handle res, long offset, void* buffer, long count);
void WritePartialResource(Handle res, long offset, const void* buffer, long count);

void SetResourceSize(Handle res, long size);

ResFileRefNum FSOpenResFile(const FSRef* ref, SInt8 permission);
void FSCreateResFile(const FSRef* parentDir, UniCharCount nameLength, const UniChar* name,
	FSCatalogInfoBitmap whichInfo, const FSCatalogInfo* catalolgInfo, FSRef* newRef, FSSpecPtr* newSpec);
OSErr FSCreateResourceFile(const FSRef* parentDir, UniCharCount nameLength, const UniChar* name,
	FSCatalogInfoBitmap whichInfo, const FSCatalogInfo* catalolgInfo,
	UniCharCount forkNameLength, const UniChar* forkName,
	FSRef* newRef, FSSpecPtr* newSpec);
OSErr FSCreateResourceFork(const FSRef* ref, UniCharCount forkNameLength, const UniChar* forkName, UInt32 flags);
OSErr FSOpenResourceFile(const FSRef* ref, UniCharCount forkNameLength, const UniChar* forkName, SInt8 permissions, ResFileRefNum* refNum);

// TODO: More 32-bit only crap

#ifdef __cplusplus
}
#endif

#endif
