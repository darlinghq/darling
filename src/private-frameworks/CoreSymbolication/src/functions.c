/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel
    
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

#include <stdio.h>
#include <stdlib.h>

#include <CoreSymbolication/CoreSymbolication.h>

static int verbose = 0;
__attribute__((constructor)) static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}
void* CSAddressSetAdd(void) { if (verbose) puts("STUB: CSAddressSetAdd called"); return NULL; }
void* CSAddressSetCreate(void) { if (verbose) puts("STUB: CSAddressSetCreate called"); return NULL; }
void* CSArchitectureGetArchitectureForName(void) { if (verbose) puts("STUB: CSArchitectureGetArchitectureForName called"); return NULL; }
void* CSArchitectureGetCurrent(void) { if (verbose) puts("STUB: CSArchitectureGetCurrent called"); return NULL; }
void* CSArchitectureGetFamily(void) { if (verbose) puts("STUB: CSArchitectureGetFamily called"); return NULL; }
void* CSArchitectureGetFamilyName(void) { if (verbose) puts("STUB: CSArchitectureGetFamilyName called"); return NULL; }
void* CSArchitectureIs32Bit(void) { if (verbose) puts("STUB: CSArchitectureIs32Bit called"); return NULL; }
int CSArchitectureIs64Bit(int architecture) { if (verbose) puts("STUB: CSArchitectureIs64Bit called"); return 0; }
void* CSArchitectureIsArm(void) { if (verbose) puts("STUB: CSArchitectureIsArm called"); return NULL; }
void* CSArchitectureIsArm64(void) { if (verbose) puts("STUB: CSArchitectureIsArm64 called"); return NULL; }
void* CSArchitectureIsBigEndian(void) { if (verbose) puts("STUB: CSArchitectureIsBigEndian called"); return NULL; }
void* CSArchitectureIsI386(void) { if (verbose) puts("STUB: CSArchitectureIsI386 called"); return NULL; }
void* CSArchitectureIsLittleEndian(void) { if (verbose) puts("STUB: CSArchitectureIsLittleEndian called"); return NULL; }
void* CSArchitectureIsX86_64(void) { if (verbose) puts("STUB: CSArchitectureIsX86_64 called"); return NULL; }
void* CSArchitectureMatchesArchitecture(void) { if (verbose) puts("STUB: CSArchitectureMatchesArchitecture called"); return NULL; }
void* CSCopyDescription(void) { if (verbose) puts("STUB: CSCopyDescription called"); return NULL; }
void* CSCopyDescriptionWithIndent(void) { if (verbose) puts("STUB: CSCopyDescriptionWithIndent called"); return NULL; }
void* CSEqual(void) { if (verbose) puts("STUB: CSEqual called"); return NULL; }
void* CSExceptionSafeThreadRunBlock(void) { if (verbose) puts("STUB: CSExceptionSafeThreadRunBlock called"); return NULL; }
void* CSGetRetainCount(void) { if (verbose) puts("STUB: CSGetRetainCount called"); return NULL; }
void* CSIsNull(void* arg0) { if (verbose) puts("STUB: CSIsNull called"); return NULL; }
void* CSMMapArchiveCacheCopyMMapArchive(void) { if (verbose) puts("STUB: CSMMapArchiveCacheCopyMMapArchive called"); return NULL; }
void* CSMMapArchiveCacheReleaseMMapArchive(void) { if (verbose) puts("STUB: CSMMapArchiveCacheReleaseMMapArchive called"); return NULL; }
void* CSMMapArchiveCacheSetShouldStoreToDaemon(void) { if (verbose) puts("STUB: CSMMapArchiveCacheSetShouldStoreToDaemon called"); return NULL; }
void* CSRangeContainsRange(void) { if (verbose) puts("STUB: CSRangeContainsRange called"); return NULL; }
void* CSRangeIntersectsRange(void) { if (verbose) puts("STUB: CSRangeIntersectsRange called"); return NULL; }
void* CSRegionCopyDescriptionWithIndent(void) { if (verbose) puts("STUB: CSRegionCopyDescriptionWithIndent called"); return NULL; }
void* CSRegionForeachSourceInfo(void) { if (verbose) puts("STUB: CSRegionForeachSourceInfo called"); return NULL; }
void CSRegionForeachSymbol(CSRegionRef region, void (^handler)(CSSymbolRef)) { if (verbose) puts("STUB: CSRegionForeachSymbol called"); }
const char* CSRegionGetName(CSSegmentRef seg) { if (verbose) puts("STUB: CSRegionGetName called"); return NULL; };
CSRange CSRegionGetRange(CSSegmentRef seg) { if (verbose) puts("STUB: CSRegionGetRange called"); return (CFRange){0}; }
void* CSRegionGetSymbolOwner(void) { if (verbose) puts("STUB: CSRegionGetSymbolOwner called"); return NULL; }
void* CSRegionGetSymbolicator(void) { if (verbose) puts("STUB: CSRegionGetSymbolicator called"); return NULL; }
void CSRelease(void* ref) { if (verbose) puts("STUB: CSRelease called"); }
void* CSRetain(void* ref) { if (verbose) puts("STUB: CSRetain called"); return NULL; }
void* CSSectionGetSegment(void) { if (verbose) puts("STUB: CSSectionGetSegment called"); return NULL; }
void* CSSegmentForeachSection(void) { if (verbose) puts("STUB: CSSegmentForeachSection called"); return NULL; }
void* CSShow(void) { if (verbose) puts("STUB: CSShow called"); return NULL; }
void* CSSignatureAddSegment(void) { if (verbose) puts("STUB: CSSignatureAddSegment called"); return NULL; }
void* CSSignatureAllocateSegments(void) { if (verbose) puts("STUB: CSSignatureAllocateSegments called"); return NULL; }
void* CSSignatureCopy(void) { if (verbose) puts("STUB: CSSignatureCopy called"); return NULL; }
void* CSSignatureEncodeSymbolOwner(void) { if (verbose) puts("STUB: CSSignatureEncodeSymbolOwner called"); return NULL; }
void* CSSignatureEncodeSymbolicator(void) { if (verbose) puts("STUB: CSSignatureEncodeSymbolicator called"); return NULL; }
void* CSSignatureFreeSegments(void) { if (verbose) puts("STUB: CSSignatureFreeSegments called"); return NULL; }
void* CSSourceInfoCopyDescriptionWithIndent(void) { if (verbose) puts("STUB: CSSourceInfoCopyDescriptionWithIndent called"); return NULL; }
void* CSSourceInfoGetColumn(void) { if (verbose) puts("STUB: CSSourceInfoGetColumn called"); return NULL; }
void* CSSourceInfoGetFilename(void) { if (verbose) puts("STUB: CSSourceInfoGetFilename called"); return NULL; }
int CSSourceInfoGetLineNumber(CSSourceInfoRef info) { if (verbose) puts("STUB: CSSourceInfoGetLineNumber called"); return 0; }
const char* CSSourceInfoGetPath(CSSourceInfoRef info) { if (verbose) puts("STUB: CSSourceInfoGetPath called"); return NULL; }
void* CSSourceInfoGetRange(void) { if (verbose) puts("STUB: CSSourceInfoGetRange called"); return NULL; }
void* CSSourceInfoGetRegion(void) { if (verbose) puts("STUB: CSSourceInfoGetRegion called"); return NULL; }
void* CSSourceInfoGetSymbol(void) { if (verbose) puts("STUB: CSSourceInfoGetSymbol called"); return NULL; }
void* CSSourceInfoGetSymbolOwner(void) { if (verbose) puts("STUB: CSSourceInfoGetSymbolOwner called"); return NULL; }
void* CSSourceInfoGetSymbolicator(void) { if (verbose) puts("STUB: CSSourceInfoGetSymbolicator called"); return NULL; }
void* CSSymbolCopyDescriptionWithIndent(void) { if (verbose) puts("STUB: CSSymbolCopyDescriptionWithIndent called"); return NULL; }
void* CSSymbolForeachSourceInfo(void) { if (verbose) puts("STUB: CSSymbolForeachSourceInfo called"); return NULL; }
void* CSSymbolGetFlags(void) { if (verbose) puts("STUB: CSSymbolGetFlags called"); return NULL; }
void* CSSymbolGetInstructionData(void) { if (verbose) puts("STUB: CSSymbolGetInstructionData called"); return NULL; }
char* CSSymbolGetMangledName(CSSymbolRef symbol) { if (verbose) puts("STUB: CSSymbolGetMangledName called"); return NULL; }
char* CSSymbolGetName(CSSymbolRef symbol) { if (verbose) puts("STUB: CSSymbolGetName called"); return NULL; }
CFRange CSSymbolGetRange(CSSymbolRef symbol) { CFRange range = {}; if (verbose) puts("STUB: CSSymbolGetRange called"); return range; }
void* CSSymbolGetRegion(void) { if (verbose) puts("STUB: CSSymbolGetRegion called"); return NULL; }
void* CSSymbolGetSection(void) { if (verbose) puts("STUB: CSSymbolGetSection called"); return NULL; }
void* CSSymbolGetSegment(void) { if (verbose) puts("STUB: CSSymbolGetSegment called"); return NULL; }
CSSymbolOwnerRef CSSymbolGetSymbolOwner(CSSymbolRef symbol) { if (verbose) puts("STUB: CSSymbolGetSymbolOwner called"); return NULL; }
void* CSSymbolGetSymbolicator(void) { if (verbose) puts("STUB: CSSymbolGetSymbolicator called"); return NULL; }
void* CSSymbolIsArm(CSSymbolRef symbol) { if (verbose) puts("STUB: CSSymbolIsArm called"); return NULL; }
void* CSSymbolIsDebugMap(void) { if (verbose) puts("STUB: CSSymbolIsDebugMap called"); return NULL; }
void* CSSymbolIsDwarf(void) { if (verbose) puts("STUB: CSSymbolIsDwarf called"); return NULL; }
int CSSymbolIsDyldStub(CSSymbolRef symbol) { if (verbose) puts("STUB: CSSymbolIsDyldStub called"); return 0; }
int CSSymbolIsExternal(CSSymbolRef symbol) { if (verbose) puts("STUB: CSSymbolIsExternal called"); return 0; }
int CSSymbolIsFunction(CSSymbolRef symbol) { if (verbose) puts("STUB: CSSymbolIsFunction called"); return 0; }
void* CSSymbolIsFunctionStarts(void) { if (verbose) puts("STUB: CSSymbolIsFunctionStarts called"); return NULL; }
void* CSSymbolIsKnownLength(void) { if (verbose) puts("STUB: CSSymbolIsKnownLength called"); return NULL; }
void* CSSymbolIsMangledNameSourceDwarf(void) { if (verbose) puts("STUB: CSSymbolIsMangledNameSourceDwarf called"); return NULL; }
void* CSSymbolIsMangledNameSourceDwarfMIPSLinkage(void) { if (verbose) puts("STUB: CSSymbolIsMangledNameSourceDwarfMIPSLinkage called"); return NULL; }
void* CSSymbolIsMangledNameSourceNList(void) { if (verbose) puts("STUB: CSSymbolIsMangledNameSourceNList called"); return NULL; }
void* CSSymbolIsMerged(void) { if (verbose) puts("STUB: CSSymbolIsMerged called"); return NULL; }
void* CSSymbolIsNList(void) { if (verbose) puts("STUB: CSSymbolIsNList called"); return NULL; }
void* CSSymbolIsNameSourceDwarf(void) { if (verbose) puts("STUB: CSSymbolIsNameSourceDwarf called"); return NULL; }
void* CSSymbolIsNameSourceDwarfMIPSLinkage(void) { if (verbose) puts("STUB: CSSymbolIsNameSourceDwarfMIPSLinkage called"); return NULL; }
void* CSSymbolIsNameSourceNList(void) { if (verbose) puts("STUB: CSSymbolIsNameSourceNList called"); return NULL; }
int CSSymbolIsObjcMethod(CSSymbolRef symbol) { if (verbose) puts("STUB: CSSymbolIsObjcMethod called"); return 0; }
void* CSSymbolIsOmitFramePointer(void) { if (verbose) puts("STUB: CSSymbolIsOmitFramePointer called"); return NULL; }
void* CSSymbolIsPrivateExternal(void) { if (verbose) puts("STUB: CSSymbolIsPrivateExternal called"); return NULL; }
void* CSSymbolIsThumb(void) { if (verbose) puts("STUB: CSSymbolIsThumb called"); return NULL; }
int CSSymbolIsUnnamed(CSSymbolRef symbol) { if (verbose) puts("STUB: CSSymbolIsUnnamed called"); return 0; }
void* CSSymbolOwnerAddInContext(void) { if (verbose) puts("STUB: CSSymbolOwnerAddInContext called"); return NULL; }
void* CSSymbolOwnerCacheFlush(void) { if (verbose) puts("STUB: CSSymbolOwnerCacheFlush called"); return NULL; }
void* CSSymbolOwnerCacheGetEntryCount(void) { if (verbose) puts("STUB: CSSymbolOwnerCacheGetEntryCount called"); return NULL; }
void* CSSymbolOwnerCacheGetFlags(void) { if (verbose) puts("STUB: CSSymbolOwnerCacheGetFlags called"); return NULL; }
void* CSSymbolOwnerCacheGetMemoryLimit(void) { if (verbose) puts("STUB: CSSymbolOwnerCacheGetMemoryLimit called"); return NULL; }
void* CSSymbolOwnerCacheGetMemoryUsed(void) { if (verbose) puts("STUB: CSSymbolOwnerCacheGetMemoryUsed called"); return NULL; }
void* CSSymbolOwnerCachePrintEntries(void) { if (verbose) puts("STUB: CSSymbolOwnerCachePrintEntries called"); return NULL; }
void* CSSymbolOwnerCachePrintStats(void) { if (verbose) puts("STUB: CSSymbolOwnerCachePrintStats called"); return NULL; }
void* CSSymbolOwnerCacheResetStats(void) { if (verbose) puts("STUB: CSSymbolOwnerCacheResetStats called"); return NULL; }
void* CSSymbolOwnerCacheSetFlags(void) { if (verbose) puts("STUB: CSSymbolOwnerCacheSetFlags called"); return NULL; }
void* CSSymbolOwnerCacheSetMemoryLimit(void) { if (verbose) puts("STUB: CSSymbolOwnerCacheSetMemoryLimit called"); return NULL; }
void* CSSymbolOwnerCopyDescriptionWithIndent(void) { if (verbose) puts("STUB: CSSymbolOwnerCopyDescriptionWithIndent called"); return NULL; }
void* CSSymbolOwnerCreateSignature(void) { if (verbose) puts("STUB: CSSymbolOwnerCreateSignature called"); return NULL; }
void* CSSymbolOwnerCreateSparseSignature(void) { if (verbose) puts("STUB: CSSymbolOwnerCreateSparseSignature called"); return NULL; }
void* CSSymbolOwnerEditRelocations(void) { if (verbose) puts("STUB: CSSymbolOwnerEditRelocations called"); return NULL; }
void* CSSymbolOwnerForEachStackFrameAtAddress(void) { if (verbose) puts("STUB: CSSymbolOwnerForEachStackFrameAtAddress called"); return NULL; }
void* CSSymbolOwnerForeachRegion(void) { if (verbose) puts("STUB: CSSymbolOwnerForeachRegion called"); return NULL; }
void* CSSymbolOwnerForeachRegionWithName(void) { if (verbose) puts("STUB: CSSymbolOwnerForeachRegionWithName called"); return NULL; }
void* CSSymbolOwnerForeachSection(void) { if (verbose) puts("STUB: CSSymbolOwnerForeachSection called"); return NULL; }
void CSSymbolOwnerForeachSegment(CSSymbolOwnerRef sym_owner, void (^loop)(CSSegmentRef)) { if (verbose) puts("STUB: CSSymbolOwnerForeachSegment called"); }
void* CSSymbolOwnerForeachSourceInfo(void) { if (verbose) puts("STUB: CSSymbolOwnerForeachSourceInfo called"); return NULL; }
void CSSymbolOwnerForeachSymbol(CSSymbolOwnerRef owner, void (^symbol)(CSSymbolRef)) { if (verbose) puts("STUB: CSSymbolOwnerForeachSymbol called"); }
void CSSymbolOwnerForeachSymbolWithMangledName(CSSymbolOwnerRef owner, const char* name, void (^handler)(CSSymbolRef)) { if (verbose) puts("STUB: CSSymbolOwnerForeachSymbolWithMangledName called"); }
void CSSymbolOwnerForeachSymbolWithName(CSSymbolOwnerRef owner, const char* name, void (^handler)(CSSymbolRef)) { if (verbose) puts("STUB: CSSymbolOwnerForeachSymbolWithName called"); }
int CSSymbolOwnerGetArchitecture(CSSymbolOwnerRef owner) { if (verbose) puts("STUB: CSSymbolOwnerGetArchitecture called"); return 0; }
uintptr_t CSSymbolOwnerGetBaseAddress(CSSymbolOwnerRef owner) { if (verbose) puts("STUB: CSSymbolOwnerGetBaseAddress called"); return 0; }
const CFUUIDBytes* CSSymbolOwnerGetCFUUIDBytes(CSSymbolOwnerRef sym_owner) { if (verbose) puts("STUB: CSSymbolOwnerGetCFUUIDBytes called"); return NULL; }
void* CSSymbolOwnerGetCompatibilityVersion(void) { if (verbose) puts("STUB: CSSymbolOwnerGetCompatibilityVersion called"); return NULL; }
void* CSSymbolOwnerGetCurrentVersion(void) { if (verbose) puts("STUB: CSSymbolOwnerGetCurrentVersion called"); return NULL; }
int CSSymbolOwnerGetDataFlags(CSSymbolOwnerRef owner) { if (verbose) puts("STUB: CSSymbolOwnerGetDataFlags called"); return 0; }
void* CSSymbolOwnerGetDataTypeID(void) { if (verbose) puts("STUB: CSSymbolOwnerGetDataTypeID called"); return NULL; }
void* CSSymbolOwnerGetDsymPath(void) { if (verbose) puts("STUB: CSSymbolOwnerGetDsymPath called"); return NULL; }
void* CSSymbolOwnerGetDsymVersion(void) { if (verbose) puts("STUB: CSSymbolOwnerGetDsymVersion called"); return NULL; }
void* CSSymbolOwnerGetFlags(void) { if (verbose) puts("STUB: CSSymbolOwnerGetFlags called"); return NULL; }
void* CSSymbolOwnerGetLastModifiedTimestamp(void) { if (verbose) puts("STUB: CSSymbolOwnerGetLastModifiedTimestamp called"); return NULL; }
void* CSSymbolOwnerGetLoadTimestamp(void) { if (verbose) puts("STUB: CSSymbolOwnerGetLoadTimestamp called"); return NULL; }
char* CSSymbolOwnerGetName(CSSymbolOwnerRef owner) { if (verbose) puts("STUB: CSSymbolOwnerGetName called"); return NULL; }
char* CSSymbolOwnerGetPath(CSSymbolOwnerRef owner) { if (verbose) puts("STUB: CSSymbolOwnerGetPath called"); return NULL; }
void* CSSymbolOwnerGetPathForSymbolication(void) { if (verbose) puts("STUB: CSSymbolOwnerGetPathForSymbolication called"); return NULL; }
void* CSSymbolOwnerGetRegionCount(void) { if (verbose) puts("STUB: CSSymbolOwnerGetRegionCount called"); return NULL; }
void* CSSymbolOwnerGetRegionWithAddress(void) { if (verbose) puts("STUB: CSSymbolOwnerGetRegionWithAddress called"); return NULL; }
CSRegionRef CSSymbolOwnerGetRegionWithName(CSSymbolOwnerRef owner, char* name) { if (verbose) puts("STUB: CSSymbolOwnerGetRegionWithName called"); return NULL; }
void* CSSymbolOwnerGetSectionWithAddress(void) { if (verbose) puts("STUB: CSSymbolOwnerGetSectionWithAddress called"); return NULL; }
void* CSSymbolOwnerGetSectionWithName(void) { if (verbose) puts("STUB: CSSymbolOwnerGetSectionWithName called"); return NULL; }
void* CSSymbolOwnerGetSegmentWithAddress(void) { if (verbose) puts("STUB: CSSymbolOwnerGetSegmentWithAddress called"); return NULL; }
void* CSSymbolOwnerGetSourceInfoCount(void) { if (verbose) puts("STUB: CSSymbolOwnerGetSourceInfoCount called"); return NULL; }
void* CSSymbolOwnerGetSourceInfoWithAddress(void) { if (verbose) puts("STUB: CSSymbolOwnerGetSourceInfoWithAddress called"); return NULL; }
void* CSSymbolOwnerGetSymbolCount(void) { if (verbose) puts("STUB: CSSymbolOwnerGetSymbolCount called"); return NULL; }
CSSymbolOwnerRef CSSymbolOwnerGetSymbolWithAddress(CSSymbolOwnerRef owner, mach_vm_address_t address) { if (verbose) puts("STUB: CSSymbolOwnerGetSymbolWithAddress called"); return NULL; }
void* CSSymbolOwnerGetSymbolWithMangledName(void) { if (verbose) puts("STUB: CSSymbolOwnerGetSymbolWithMangledName called"); return NULL; }
void* CSSymbolOwnerGetSymbolWithName(void) { if (verbose) puts("STUB: CSSymbolOwnerGetSymbolWithName called"); return NULL; }
void* CSSymbolOwnerGetSymbolicator(void) { if (verbose) puts("STUB: CSSymbolOwnerGetSymbolicator called"); return NULL; }
uintptr_t CSSymbolOwnerGetTransientUserData(CSSymbolOwnerRef owner) { if (verbose) puts("STUB: CSSymbolOwnerGetTransientUserData called"); return 0; }
void* CSSymbolOwnerGetUUID(void) { if (verbose) puts("STUB: CSSymbolOwnerGetUUID called"); return NULL; }
void* CSSymbolOwnerGetUnloadTimestamp(void) { if (verbose) puts("STUB: CSSymbolOwnerGetUnloadTimestamp called"); return NULL; }
void* CSSymbolOwnerGetVersion(void) { if (verbose) puts("STUB: CSSymbolOwnerGetVersion called"); return NULL; }
void* CSSymbolOwnerIsAOut(void) { if (verbose) puts("STUB: CSSymbolOwnerIsAOut called"); return NULL; }
void* CSSymbolOwnerIsBundle(void) { if (verbose) puts("STUB: CSSymbolOwnerIsBundle called"); return NULL; }
void* CSSymbolOwnerIsCommpage(void) { if (verbose) puts("STUB: CSSymbolOwnerIsCommpage called"); return NULL; }
void* CSSymbolOwnerIsDsym(void) { if (verbose) puts("STUB: CSSymbolOwnerIsDsym called"); return NULL; }
void* CSSymbolOwnerIsDyld(void) { if (verbose) puts("STUB: CSSymbolOwnerIsDyld called"); return NULL; }
void* CSSymbolOwnerIsDyldSharedCache(void) { if (verbose) puts("STUB: CSSymbolOwnerIsDyldSharedCache called"); return NULL; }
void* CSSymbolOwnerIsDylib(void) { if (verbose) puts("STUB: CSSymbolOwnerIsDylib called"); return NULL; }
void* CSSymbolOwnerIsDylibStub(void) { if (verbose) puts("STUB: CSSymbolOwnerIsDylibStub called"); return NULL; }
void* CSSymbolOwnerIsKextBundle(void) { if (verbose) puts("STUB: CSSymbolOwnerIsKextBundle called"); return NULL; }
void* CSSymbolOwnerIsMachO(void) { if (verbose) puts("STUB: CSSymbolOwnerIsMachO called"); return NULL; }
void* CSSymbolOwnerIsMutable(void) { if (verbose) puts("STUB: CSSymbolOwnerIsMutable called"); return NULL; }
void* CSSymbolOwnerIsObjCGCSupported(void) { if (verbose) puts("STUB: CSSymbolOwnerIsObjCGCSupported called"); return NULL; }
void* CSSymbolOwnerIsObjCRetainReleaseSupported(void) { if (verbose) puts("STUB: CSSymbolOwnerIsObjCRetainReleaseSupported called"); return NULL; }
void* CSSymbolOwnerIsObject(CSSymbolOwnerRef owner) { if (verbose) puts("STUB: CSSymbolOwnerIsObject called"); return NULL; }
void* CSSymbolOwnerIsObsolete(void) { if (verbose) puts("STUB: CSSymbolOwnerIsObsolete called"); return NULL; }
void* CSSymbolOwnerIsPIE(void) { if (verbose) puts("STUB: CSSymbolOwnerIsPIE called"); return NULL; }
void* CSSymbolOwnerIsProtected(void) { if (verbose) puts("STUB: CSSymbolOwnerIsProtected called"); return NULL; }
void* CSSymbolOwnerIsRestricted(void) { if (verbose) puts("STUB: CSSymbolOwnerIsRestricted called"); return NULL; }
void* CSSymbolOwnerIsSlid(void) { if (verbose) puts("STUB: CSSymbolOwnerIsSlid called"); return NULL; }
void* CSSymbolOwnerIsStaticLibraryArchiveEntry(void) { if (verbose) puts("STUB: CSSymbolOwnerIsStaticLibraryArchiveEntry called"); return NULL; }
void* CSSymbolOwnerMakeMutableInContext(void) { if (verbose) puts("STUB: CSSymbolOwnerMakeMutableInContext called"); return NULL; }
void* CSSymbolOwnerNeedsResymbolication(void) { if (verbose) puts("STUB: CSSymbolOwnerNeedsResymbolication called"); return NULL; }
void* CSSymbolOwnerRemoveInContext(void) { if (verbose) puts("STUB: CSSymbolOwnerRemoveInContext called"); return NULL; }
void* CSSymbolOwnerSetLoadTimestamp(void) { if (verbose) puts("STUB: CSSymbolOwnerSetLoadTimestamp called"); return NULL; }
void* CSSymbolOwnerSetPath(void) { if (verbose) puts("STUB: CSSymbolOwnerSetPath called"); return NULL; }
void* CSSymbolOwnerSetPathForSymbolication(void) { if (verbose) puts("STUB: CSSymbolOwnerSetPathForSymbolication called"); return NULL; }
void* CSSymbolOwnerSetRelocationCount(void) { if (verbose) puts("STUB: CSSymbolOwnerSetRelocationCount called"); return NULL; }
void CSSymbolOwnerSetTransientUserData(CSSymbolOwnerRef owner, uint32_t generation) { if (verbose) puts("STUB: CSSymbolOwnerSetTransientUserData called"); }
void* CSSymbolOwnerSetUnloadTimestamp(void) { if (verbose) puts("STUB: CSSymbolOwnerSetUnloadTimestamp called"); return NULL; }
void* CSSymbolicatorAddSymbolOwner(void) { if (verbose) puts("STUB: CSSymbolicatorAddSymbolOwner called"); return NULL; }
void* CSSymbolicatorApplyMutableContextBlock(void) { if (verbose) puts("STUB: CSSymbolicatorApplyMutableContextBlock called"); return NULL; }
void* CSSymbolicatorCopyDescriptionWithIndent(void) { if (verbose) puts("STUB: CSSymbolicatorCopyDescriptionWithIndent called"); return NULL; }
void* CSSymbolicatorCreateSignature(void) { if (verbose) puts("STUB: CSSymbolicatorCreateSignature called"); return NULL; }
void* CSSymbolicatorCreateWithBinaryImageList(void) { if (verbose) puts("STUB: CSSymbolicatorCreateWithBinaryImageList called"); return NULL; }
void* CSSymbolicatorCreateWithBinaryImageListCountPidFlagsAndNotification(void) { if (verbose) puts("STUB: CSSymbolicatorCreateWithBinaryImageListCountPidFlagsAndNotification called"); return NULL; }
void* CSSymbolicatorCreateWithMachKernel(void) { if (verbose) puts("STUB: CSSymbolicatorCreateWithMachKernel called"); return NULL; }
void* CSSymbolicatorCreateWithMachKernelFlagsAndNotification(int arg0, void* arg1) { if (verbose) puts("STUB: CSSymbolicatorCreateWithMachKernelFlagsAndNotification called"); return NULL; }
void* CSSymbolicatorCreateWithPathAndArchitecture(void) { if (verbose) puts("STUB: CSSymbolicatorCreateWithPathAndArchitecture called"); return NULL; }
void* CSSymbolicatorCreateWithPathArchitectureFlagsAndNotification(void) { if (verbose) puts("STUB: CSSymbolicatorCreateWithPathArchitectureFlagsAndNotification called"); return NULL; }
CSSymbolicatorRef CSSymbolicatorCreateWithPid(pid_t pid) { if (verbose) puts("STUB: CSSymbolicatorCreateWithPid called"); return NULL; }
void* CSSymbolicatorCreateWithPidFlagsAndNotification(void) { if (verbose) puts("STUB: CSSymbolicatorCreateWithPidFlagsAndNotification called"); return NULL; }
void* CSSymbolicatorCreateWithSignature(void) { if (verbose) puts("STUB: CSSymbolicatorCreateWithSignature called"); return NULL; }
void* CSSymbolicatorCreateWithSignatureAndNotification(void) { if (verbose) puts("STUB: CSSymbolicatorCreateWithSignatureAndNotification called"); return NULL; }
void* CSSymbolicatorCreateWithSignatureFlagsAndNotification(void) { if (verbose) puts("STUB: CSSymbolicatorCreateWithSignatureFlagsAndNotification called"); return NULL; }
void* CSSymbolicatorCreateWithTask(void) { if (verbose) puts("STUB: CSSymbolicatorCreateWithTask called"); return NULL; }
CSSymbolicatorRef CSSymbolicatorCreateWithTaskFlagsAndNotification(task_t task, uint32_t flags, void (^handler)(uint32_t notification_type, CSNotificationData data)) { if (verbose) puts("STUB: CSSymbolicatorCreateWithTaskFlagsAndNotification called"); return NULL; }
void* CSSymbolicatorCreateWithTaskPidFlagsAndNotification(void) { if (verbose) puts("STUB: CSSymbolicatorCreateWithTaskPidFlagsAndNotification called"); return NULL; }
void* CSSymbolicatorCreateWithURLAndArchitecture(void) { if (verbose) puts("STUB: CSSymbolicatorCreateWithURLAndArchitecture called"); return NULL; }
void* CSSymbolicatorCreateWithURLArchitectureFlagsAndNotification(void) { if (verbose) puts("STUB: CSSymbolicatorCreateWithURLArchitectureFlagsAndNotification called"); return NULL; }
void* CSSymbolicatorFixupFrames(void) { if (verbose) puts("STUB: CSSymbolicatorFixupFrames called"); return NULL; }
void* CSSymbolicatorForceFullSymbolExtraction(void) { if (verbose) puts("STUB: CSSymbolicatorForceFullSymbolExtraction called"); return NULL; }
void* CSSymbolicatorForeachRegionAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorForeachRegionAtTime called"); return NULL; }
void* CSSymbolicatorForeachRegionWithNameAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorForeachRegionWithNameAtTime called"); return NULL; }
void* CSSymbolicatorForeachSectionAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorForeachSectionAtTime called"); return NULL; }
void* CSSymbolicatorForeachSegmentAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorForeachSegmentAtTime called"); return NULL; }
void* CSSymbolicatorForeachSharedCache(void) { if (verbose) puts("STUB: CSSymbolicatorForeachSharedCache called"); return NULL; }
void* CSSymbolicatorForeachSharedCacheSymbolicatorWithFlagsAndNotification(void) { if (verbose) puts("STUB: CSSymbolicatorForeachSharedCacheSymbolicatorWithFlagsAndNotification called"); return NULL; }
void* CSSymbolicatorForeachSourceInfoAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorForeachSourceInfoAtTime called"); return NULL; }
void* CSSymbolicatorForeachSymbolAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorForeachSymbolAtTime called"); return NULL; }
int CSSymbolicatorForeachSymbolOwnerAtTime(CSSymbolicatorRef symbolicator, int time, void (^handler)(CSSymbolOwnerRef t)) { if (verbose) puts("STUB: CSSymbolicatorForeachSymbolOwnerAtTime called"); return 0; }
void* CSSymbolicatorForeachSymbolOwnerWithCFUUIDBytesAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorForeachSymbolOwnerWithCFUUIDBytesAtTime called"); return NULL; }
int CSSymbolicatorForeachSymbolOwnerWithFlagsAtTime(CSSymbolicatorRef symbolicator, int flags, int time, void (^handler)(CSSymbolOwnerRef t)) { if (verbose) puts("STUB: CSSymbolicatorForeachSymbolOwnerWithFlagsAtTime called"); return 0; }
int CSSymbolicatorForeachSymbolOwnerWithNameAtTime(CSSymbolicatorRef symbolicator, const char* name, int time, void (^handler)(CSSymbolOwnerRef t)) { if (verbose) puts("STUB: CSSymbolicatorForeachSymbolOwnerWithNameAtTime called"); return 0; }
int CSSymbolicatorForeachSymbolOwnerWithPathAtTime(CSSymbolicatorRef symbolicator, const char* path, int time, void (^handler)(CSSymbolOwnerRef t)) { if (verbose) puts("STUB: CSSymbolicatorForeachSymbolOwnerWithPathAtTime called"); return 0; }
void* CSSymbolicatorForeachSymbolOwnerWithUUIDAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorForeachSymbolOwnerWithUUIDAtTime called"); return NULL; }
void CSSymbolicatorForeachSymbolWithMangledNameAtTime(CSSymbolicatorRef symbolicator, const char* name, int time, void (^handler)(CSSymbolRef t)) { if (verbose) puts("STUB: CSSymbolicatorForeachSymbolWithMangledNameAtTime called"); }
void CSSymbolicatorForeachSymbolWithNameAtTime(CSSymbolicatorRef symbolicator, const char* name, int time, void (^handler)(CSSymbolRef t)) { if (verbose) puts("STUB: CSSymbolicatorForeachSymbolWithNameAtTime called"); }
void* CSSymbolicatorForeachSymbolicatorWithPath(void) { if (verbose) puts("STUB: CSSymbolicatorForeachSymbolicatorWithPath called"); return NULL; }
void* CSSymbolicatorForeachSymbolicatorWithPathFlagsAndNotification(void) { if (verbose) puts("STUB: CSSymbolicatorForeachSymbolicatorWithPathFlagsAndNotification called"); return NULL; }
void* CSSymbolicatorForeachSymbolicatorWithURL(void) { if (verbose) puts("STUB: CSSymbolicatorForeachSymbolicatorWithURL called"); return NULL; }
void* CSSymbolicatorForeachSymbolicatorWithURLFlagsAndNotification(void) { if (verbose) puts("STUB: CSSymbolicatorForeachSymbolicatorWithURLFlagsAndNotification called"); return NULL; }
void* CSSymbolicatorGetAOutSymbolOwner(int arg0) { if (verbose) puts("STUB: CSSymbolicatorGetAOutSymbolOwner called"); return NULL; }
int CSSymbolicatorGetArchitecture(CSSymbolicatorRef symbolicator) { if (verbose) puts("STUB: CSSymbolicatorGetArchitecture called"); return 0; }
void* CSSymbolicatorGetFlagsForDebugMapOnlyData(void) { if (verbose) puts("STUB: CSSymbolicatorGetFlagsForDebugMapOnlyData called"); return NULL; }
void* CSSymbolicatorGetFlagsForDsymOnlyData(void) { if (verbose) puts("STUB: CSSymbolicatorGetFlagsForDsymOnlyData called"); return NULL; }
void* CSSymbolicatorGetFlagsForDwarfOnlyData(void) { if (verbose) puts("STUB: CSSymbolicatorGetFlagsForDwarfOnlyData called"); return NULL; }
void* CSSymbolicatorGetFlagsForFunctionStartsOnlyData(void) { if (verbose) puts("STUB: CSSymbolicatorGetFlagsForFunctionStartsOnlyData called"); return NULL; }
void* CSSymbolicatorGetFlagsForNListOnlyData(void) { if (verbose) puts("STUB: CSSymbolicatorGetFlagsForNListOnlyData called"); return NULL; }
void* CSSymbolicatorGetFlagsForNoSymbolOrSourceInfoData(void) { if (verbose) puts("STUB: CSSymbolicatorGetFlagsForNoSymbolOrSourceInfoData called"); return NULL; }
pid_t CSSymbolicatorGetPid(CSSymbolicatorRef symbolicator) { if (verbose) puts("STUB: CSSymbolicatorGetPid called"); return 0; }
void* CSSymbolicatorGetRegionCountAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorGetRegionCountAtTime called"); return NULL; }
void* CSSymbolicatorGetRegionWithAddressAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorGetRegionWithAddressAtTime called"); return NULL; }
void* CSSymbolicatorGetRegionWithNameAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorGetRegionWithNameAtTime called"); return NULL; }
void* CSSymbolicatorGetSectionWithAddressAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorGetSectionWithAddressAtTime called"); return NULL; }
void* CSSymbolicatorGetSegmentWithAddressAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorGetSegmentWithAddressAtTime called"); return NULL; }
void* CSSymbolicatorGetSharedCacheSlide(void) { if (verbose) puts("STUB: CSSymbolicatorGetSharedCacheSlide called"); return NULL; }
void* CSSymbolicatorGetSharedCacheUUID(void) { if (verbose) puts("STUB: CSSymbolicatorGetSharedCacheUUID called"); return NULL; }
void* CSSymbolicatorGetSourceInfoCountAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorGetSourceInfoCountAtTime called"); return NULL; }
CSSourceInfoRef CSSymbolicatorGetSourceInfoWithAddressAtTime(CSSymbolicatorRef symbolicator, mach_vm_address_t address, time_t time) { if (verbose) puts("STUB: CSSymbolicatorGetSourceInfoWithAddressAtTime called"); return NULL; }
void* CSSymbolicatorGetSymbolCountAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorGetSymbolCountAtTime called"); return NULL; }
void* CSSymbolicatorGetSymbolOwner(void) { if (verbose) puts("STUB: CSSymbolicatorGetSymbolOwner called"); return NULL; }
void* CSSymbolicatorGetSymbolOwnerCountAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorGetSymbolOwnerCountAtTime called"); return NULL; }
CSSymbolOwnerRef CSSymbolicatorGetSymbolOwnerWithAddressAtTime(CSSymbolicatorRef symbolicator, mach_vm_address_t address, int time) { if (verbose) puts("STUB: CSSymbolicatorGetSymbolOwnerWithAddressAtTime called"); return NULL; }
void* CSSymbolicatorGetSymbolOwnerWithCFUUIDBytesAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorGetSymbolOwnerWithCFUUIDBytesAtTime called"); return NULL; }
void* CSSymbolicatorGetSymbolOwnerWithNameAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorGetSymbolOwnerWithNameAtTime called"); return NULL; }
CSSymbolOwnerRef CSSymbolicatorGetSymbolOwnerWithUUIDAtTime(CSSymbolicatorRef symbolicator, CFUUIDRef uuid, int time) { if (verbose) puts("STUB: CSSymbolicatorGetSymbolOwnerWithUUIDAtTime called"); return NULL; }
CSSymbolRef CSSymbolicatorGetSymbolWithAddressAtTime(CSSymbolicatorRef symbolicator, mach_vm_address_t addr, time_t time) { if (verbose) puts("STUB: CSSymbolicatorGetSymbolWithAddressAtTime called"); return NULL; }
void* CSSymbolicatorGetSymbolWithMangledNameAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorGetSymbolWithMangledNameAtTime called"); return NULL; }
void* CSSymbolicatorGetSymbolWithMangledNameFromSymbolOwnerWithNameAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorGetSymbolWithMangledNameFromSymbolOwnerWithNameAtTime called"); return NULL; }
void* CSSymbolicatorGetSymbolWithNameAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorGetSymbolWithNameAtTime called"); return NULL; }
void* CSSymbolicatorGetSymbolWithNameFromSymbolOwnerWithNameAtTime(void) { if (verbose) puts("STUB: CSSymbolicatorGetSymbolWithNameFromSymbolOwnerWithNameAtTime called"); return NULL; }
task_t CSSymbolicatorGetTask(CSSymbolicatorRef symbolicator) { if (verbose) puts("STUB: CSSymbolicatorGetTask called"); return 0; }
Boolean CSSymbolicatorIsKernelSymbolicator(CSSymbolicatorRef symbolicator) { if (verbose) puts("STUB: CSSymbolicatorIsKernelSymbolicator called"); return 0; }
void* CSSymbolicatorIsTaskTranslated(void) { if (verbose) puts("STUB: CSSymbolicatorIsTaskTranslated called"); return NULL; }
void* CSSymbolicatorIsTaskValid(void) { if (verbose) puts("STUB: CSSymbolicatorIsTaskValid called"); return NULL; }
void* CSSymbolicatorResymbolicate(void) { if (verbose) puts("STUB: CSSymbolicatorResymbolicate called"); return NULL; }
void* CSSymbolicatorResymbolicateFail(void) { if (verbose) puts("STUB: CSSymbolicatorResymbolicateFail called"); return NULL; }
void* CSSymbolicatorResymbolicateFromDebugSymbolsInfo(void) { if (verbose) puts("STUB: CSSymbolicatorResymbolicateFromDebugSymbolsInfo called"); return NULL; }
void* CSSymbolicatorSetForceGlobalSafeMachVMReads(void) { if (verbose) puts("STUB: CSSymbolicatorSetForceGlobalSafeMachVMReads called"); return NULL; }
void CSSymbolicatorSubscribeToTaskMainNotification(CSSymbolicatorRef symbolicator) { if (verbose) puts("STUB: CSSymbolicatorSubscribeToTaskMainNotification called"); }
void* CSTotalBytesMapped(void) { if (verbose) puts("STUB: CSTotalBytesMapped called"); return NULL; }
void* CSUUIDCFUUIDBytesToPath(void) { if (verbose) puts("STUB: CSUUIDCFUUIDBytesToPath called"); return NULL; }
void* CSUUIDCFUUIDBytesToString(void) { if (verbose) puts("STUB: CSUUIDCFUUIDBytesToString called"); return NULL; }
void* CSUUIDStringToCFUUIDBytes(void) { if (verbose) puts("STUB: CSUUIDStringToCFUUIDBytes called"); return NULL; }
void* clear_mapped_memory(void) { if (verbose) puts("STUB: clear_mapped_memory called"); return NULL; }
void* create_mapped_memory_cache_for_task(void) { if (verbose) puts("STUB: create_mapped_memory_cache_for_task called"); return NULL; }
void* create_sampling_context_for_task(void) { if (verbose) puts("STUB: create_sampling_context_for_task called"); return NULL; }
void* create_sampling_context_for_task_and_pid(void) { if (verbose) puts("STUB: create_sampling_context_for_task_and_pid called"); return NULL; }
void* create_sampling_context_for_task_pid_symbolicator(void) { if (verbose) puts("STUB: create_sampling_context_for_task_pid_symbolicator called"); return NULL; }
void* demangle(void) { if (verbose) puts("STUB: demangle called"); return NULL; }
void* destroy_mapped_memory_cache(void) { if (verbose) puts("STUB: destroy_mapped_memory_cache called"); return NULL; }
void* destroy_sampling_context(void) { if (verbose) puts("STUB: destroy_sampling_context called"); return NULL; }
void* dispatch_queue_name_for_serial_number(void) { if (verbose) puts("STUB: dispatch_queue_name_for_serial_number called"); return NULL; }
void* find_node(void) { if (verbose) puts("STUB: find_node called"); return NULL; }
void* get_remote_thread_dispatch_queue(void) { if (verbose) puts("STUB: get_remote_thread_dispatch_queue called"); return NULL; }
void* map_new_node(void) { if (verbose) puts("STUB: map_new_node called"); return NULL; }
void* mapped_memory_read(void) { if (verbose) puts("STUB: mapped_memory_read called"); return NULL; }
void* mapped_memory_read_pointer(void) { if (verbose) puts("STUB: mapped_memory_read_pointer called"); return NULL; }
void* next_node(void) { if (verbose) puts("STUB: next_node called"); return NULL; }
void* sample_remote_thread(void) { if (verbose) puts("STUB: sample_remote_thread called"); return NULL; }
void* sample_remote_thread_with_dispatch_queue(void) { if (verbose) puts("STUB: sample_remote_thread_with_dispatch_queue called"); return NULL; }
void* sampling_context_clear_cache(void) { if (verbose) puts("STUB: sampling_context_clear_cache called"); return NULL; }
void* task_is_64bit(void) { if (verbose) puts("STUB: task_is_64bit called"); return NULL; }
void* thread_name_for_thread_port(void) { if (verbose) puts("STUB: thread_name_for_thread_port called"); return NULL; }
