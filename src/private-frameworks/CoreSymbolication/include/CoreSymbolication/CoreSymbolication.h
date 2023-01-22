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

#ifndef __CORESYMBOLICATION_CORESYMBOLICATION_H__
#define __CORESYMBOLICATION_CORESYMBOLICATION_H__

#include <CoreFoundation/CoreFoundation.h>
#include <mach/machine/vm_types.h>
#include <mach/mach_types.h>
#include <sys/types.h>

typedef CFRange CSRange;

typedef void* CSSymbolRef;
typedef void* CSSymbolicatorRef;
typedef void* CSSymbolOwnerRef;
//typedef void (CSSymbolIterator)(CSSymbolRef);
typedef void (^CSSymbolIterator)(CSSymbolRef);
typedef void* CSRegionRef;
typedef void* CSSegmentRef;
typedef void* CSSourceInfoRef;

typedef struct {
    int value;
} CSPing;

typedef struct {
    char *symbolOwner;
} CSDyldLoad;

typedef struct {
    CSPing ping;
    CSDyldLoad dyldLoad;
} CSUnknown;

typedef struct {
    CSSymbolicatorRef symbolicator;
    CSUnknown u;
} CSNotificationData;

#define kCSNotificationTaskMain 1 << 0
#define kCSNotificationPing 1 << 1
#define kCSNotificationInitialized 1 << 2
#define kCSNotificationDyldLoad 1 << 3
#define kCSNotificationDyldUnload 1 << 4
#define kCSNotificationTimeout 1 << 5
#define kCSNotificationTaskExit 1 << 6
#define kCSNotificationFini 1 << 7

#define kCSNow 0xFFFFFFFF
#define kCSNull NULL

#define kCSSymbolicatorUseSlidKernelAddresses 1 << 0
#define kCSSymbolicatorDisallowDsymData 1 << 1
#define kCSSymbolicatorTrackDyldActivity 1 << 2

#define kCSSymbolicatorDefaultCreateFlags 0

#define kCSSymbolOwnerDataFoundDsym 1 << 0
#define kCSSymbolOwnerDataEmpty 1 << 1
#define kCSSymbolOwnerIsAOut 1 << 2

void* CSAddressSetAdd(void);
void* CSAddressSetCreate(void);
void* CSArchitectureGetArchitectureForName(void);
void* CSArchitectureGetCurrent(void);
void* CSArchitectureGetFamily(void);
void* CSArchitectureGetFamilyName(void);
void* CSArchitectureIs32Bit(void);
int CSArchitectureIs64Bit(int architecture);
void* CSArchitectureIsArm(void);
void* CSArchitectureIsArm64(void);
void* CSArchitectureIsBigEndian(void);
void* CSArchitectureIsI386(void);
void* CSArchitectureIsLittleEndian(void);
void* CSArchitectureIsX86_64(void);
void* CSArchitectureMatchesArchitecture(void);
void* CSCopyDescription(void);
void* CSCopyDescriptionWithIndent(void);
void* CSEqual(void);
void* CSExceptionSafeThreadRunBlock(void);
void* CSGetRetainCount(void);
void* CSIsNull(void* arg0);
void* CSMMapArchiveCacheCopyMMapArchive(void);
void* CSMMapArchiveCacheReleaseMMapArchive(void);
void* CSMMapArchiveCacheSetShouldStoreToDaemon(void);
void* CSRangeContainsRange(void);
void* CSRangeIntersectsRange(void);
void* CSRegionCopyDescriptionWithIndent(void);
void* CSRegionForeachSourceInfo(void);
void CSRegionForeachSymbol(CSRegionRef region, void (^handler)(CSSymbolRef));
const char* CSRegionGetName(CSSegmentRef);
CSRange CSRegionGetRange(CSSegmentRef);
void* CSRegionGetSymbolOwner(void);
void* CSRegionGetSymbolicator(void);
void CSRelease(void* symbolicator);
void* CSRetain(void*);
void* CSSectionGetSegment(void);
void* CSSegmentForeachSection(void);
void* CSShow(void);
void* CSSignatureAddSegment(void);
void* CSSignatureAllocateSegments(void);
void* CSSignatureCopy(void);
void* CSSignatureEncodeSymbolOwner(void);
void* CSSignatureEncodeSymbolicator(void);
void* CSSignatureFreeSegments(void);
void* CSSourceInfoCopyDescriptionWithIndent(void);
void* CSSourceInfoGetColumn(void);
void* CSSourceInfoGetFilename(void);
int CSSourceInfoGetLineNumber(CSSourceInfoRef);
const char* CSSourceInfoGetPath(CSSourceInfoRef);
void* CSSourceInfoGetRange(void);
void* CSSourceInfoGetRegion(void);
void* CSSourceInfoGetSymbol(void);
void* CSSourceInfoGetSymbolOwner(void);
void* CSSourceInfoGetSymbolicator(void);
void* CSSymbolCopyDescriptionWithIndent(void);
void* CSSymbolForeachSourceInfo(void);
void* CSSymbolGetFlags(void);
void* CSSymbolGetInstructionData(void);
char* CSSymbolGetMangledName(CSSymbolRef symbol);
char* CSSymbolGetName(CSSymbolRef symbol);
CFRange CSSymbolGetRange(CSSymbolRef symbol);
void* CSSymbolGetRegion(void);
void* CSSymbolGetSection(void);
void* CSSymbolGetSegment(void);
CSSymbolOwnerRef CSSymbolGetSymbolOwner(CSSymbolRef symbol);
void* CSSymbolGetSymbolicator(void);
void* CSSymbolIsArm(CSSymbolRef symbol);
void* CSSymbolIsDebugMap(void);
void* CSSymbolIsDwarf(void);
int CSSymbolIsDyldStub(CSSymbolRef symbol);
int CSSymbolIsExternal(CSSymbolRef symbol);
int CSSymbolIsFunction(CSSymbolRef symbol);
void* CSSymbolIsFunctionStarts(void);
void* CSSymbolIsKnownLength(void);
void* CSSymbolIsMangledNameSourceDwarf(void);
void* CSSymbolIsMangledNameSourceDwarfMIPSLinkage(void);
void* CSSymbolIsMangledNameSourceNList(void);
void* CSSymbolIsMerged(void);
void* CSSymbolIsNList(void);
void* CSSymbolIsNameSourceDwarf(void);
void* CSSymbolIsNameSourceDwarfMIPSLinkage(void);
void* CSSymbolIsNameSourceNList(void);
int CSSymbolIsObjcMethod(CSSymbolRef symbol);
void* CSSymbolIsOmitFramePointer(void);
void* CSSymbolIsPrivateExternal(void);
void* CSSymbolIsThumb(void);
int CSSymbolIsUnnamed(CSSymbolRef symbol);
void* CSSymbolOwnerAddInContext(void);
void* CSSymbolOwnerCacheFlush(void);
void* CSSymbolOwnerCacheGetEntryCount(void);
void* CSSymbolOwnerCacheGetFlags(void);
void* CSSymbolOwnerCacheGetMemoryLimit(void);
void* CSSymbolOwnerCacheGetMemoryUsed(void);
void* CSSymbolOwnerCachePrintEntries(void);
void* CSSymbolOwnerCachePrintStats(void);
void* CSSymbolOwnerCacheResetStats(void);
void* CSSymbolOwnerCacheSetFlags(void);
void* CSSymbolOwnerCacheSetMemoryLimit(void);
void* CSSymbolOwnerCopyDescriptionWithIndent(void);
void* CSSymbolOwnerCreateSignature(void);
void* CSSymbolOwnerCreateSparseSignature(void);
void* CSSymbolOwnerEditRelocations(void);
void* CSSymbolOwnerForEachStackFrameAtAddress(void);
void* CSSymbolOwnerForeachRegion(void);
void* CSSymbolOwnerForeachRegionWithName(void);
void* CSSymbolOwnerForeachSection(void);
void CSSymbolOwnerForeachSegment(CSSymbolOwnerRef, void (^)(CSSegmentRef));
void* CSSymbolOwnerForeachSourceInfo(void);
void CSSymbolOwnerForeachSymbol(CSSymbolOwnerRef owner, void (^symbol)(CSSymbolRef));
void CSSymbolOwnerForeachSymbolWithMangledName(CSSymbolOwnerRef owner, const char* name, void (^handler)(CSSymbolRef));
void CSSymbolOwnerForeachSymbolWithName(CSSymbolOwnerRef owner, const char* name, void (^handler)(CSSymbolRef));
int CSSymbolOwnerGetArchitecture(CSSymbolOwnerRef owner);
uintptr_t CSSymbolOwnerGetBaseAddress(CSSymbolOwnerRef owner);
const CFUUIDBytes* CSSymbolOwnerGetCFUUIDBytes(CSSymbolOwnerRef);
void* CSSymbolOwnerGetCompatibilityVersion(void);
void* CSSymbolOwnerGetCurrentVersion(void);
int CSSymbolOwnerGetDataFlags(CSSymbolOwnerRef owner);
void* CSSymbolOwnerGetDataTypeID(void);
void* CSSymbolOwnerGetDsymPath(void);
void* CSSymbolOwnerGetDsymVersion(void);
void* CSSymbolOwnerGetFlags(void);
void* CSSymbolOwnerGetLastModifiedTimestamp(void);
void* CSSymbolOwnerGetLoadTimestamp(void);
char* CSSymbolOwnerGetName(CSSymbolOwnerRef owner);
char* CSSymbolOwnerGetPath(CSSymbolOwnerRef owner);
void* CSSymbolOwnerGetPathForSymbolication(void);
void* CSSymbolOwnerGetRegionCount(void);
void* CSSymbolOwnerGetRegionWithAddress(void);
CSRegionRef CSSymbolOwnerGetRegionWithName(CSSymbolOwnerRef owner, char* name);
void* CSSymbolOwnerGetSectionWithAddress(void);
void* CSSymbolOwnerGetSectionWithName(void);
void* CSSymbolOwnerGetSegmentWithAddress(void);
void* CSSymbolOwnerGetSourceInfoCount(void);
void* CSSymbolOwnerGetSourceInfoWithAddress(void);
void* CSSymbolOwnerGetSymbolCount(void);
CSSymbolOwnerRef CSSymbolOwnerGetSymbolWithAddress(CSSymbolOwnerRef owner, mach_vm_address_t address);
void* CSSymbolOwnerGetSymbolWithMangledName(void);
void* CSSymbolOwnerGetSymbolWithName(void);
void* CSSymbolOwnerGetSymbolicator(void);
uintptr_t CSSymbolOwnerGetTransientUserData(CSSymbolOwnerRef owner);
void* CSSymbolOwnerGetUUID(void);
void* CSSymbolOwnerGetUnloadTimestamp(void);
void* CSSymbolOwnerGetVersion(void);
void* CSSymbolOwnerIsAOut(void);
void* CSSymbolOwnerIsBundle(void);
void* CSSymbolOwnerIsCommpage(void);
void* CSSymbolOwnerIsDsym(void);
void* CSSymbolOwnerIsDyld(void);
void* CSSymbolOwnerIsDyldSharedCache(void);
void* CSSymbolOwnerIsDylib(void);
void* CSSymbolOwnerIsDylibStub(void);
void* CSSymbolOwnerIsKextBundle(void);
void* CSSymbolOwnerIsMachO(void);
void* CSSymbolOwnerIsMutable(void);
void* CSSymbolOwnerIsObjCGCSupported(void);
void* CSSymbolOwnerIsObjCRetainReleaseSupported(void);
void* CSSymbolOwnerIsObject(CSSymbolOwnerRef owner);
void* CSSymbolOwnerIsObsolete(void);
void* CSSymbolOwnerIsPIE(void);
void* CSSymbolOwnerIsProtected(void);
void* CSSymbolOwnerIsRestricted(void);
void* CSSymbolOwnerIsSlid(void);
void* CSSymbolOwnerIsStaticLibraryArchiveEntry(void);
void* CSSymbolOwnerMakeMutableInContext(void);
void* CSSymbolOwnerNeedsResymbolication(void);
void* CSSymbolOwnerRemoveInContext(void);
void* CSSymbolOwnerSetLoadTimestamp(void);
void* CSSymbolOwnerSetPath(void);
void* CSSymbolOwnerSetPathForSymbolication(void);
void* CSSymbolOwnerSetRelocationCount(void);
void CSSymbolOwnerSetTransientUserData(CSSymbolOwnerRef owner, uint32_t generation);
void* CSSymbolOwnerSetUnloadTimestamp(void);
void* CSSymbolicatorAddSymbolOwner(void);
void* CSSymbolicatorApplyMutableContextBlock(void);
void* CSSymbolicatorCopyDescriptionWithIndent(void);
void* CSSymbolicatorCreateSignature(void);
void* CSSymbolicatorCreateWithBinaryImageList(void);
void* CSSymbolicatorCreateWithBinaryImageListCountPidFlagsAndNotification(void);
void* CSSymbolicatorCreateWithMachKernel(void);
void* CSSymbolicatorCreateWithMachKernelFlagsAndNotification(int arg0, void* arg1);
void* CSSymbolicatorCreateWithPathAndArchitecture(void);
void* CSSymbolicatorCreateWithPathArchitectureFlagsAndNotification(void);
CSSymbolicatorRef CSSymbolicatorCreateWithPid(pid_t);
void* CSSymbolicatorCreateWithPidFlagsAndNotification(void);
void* CSSymbolicatorCreateWithSignature(void);
void* CSSymbolicatorCreateWithSignatureAndNotification(void);
void* CSSymbolicatorCreateWithSignatureFlagsAndNotification(void);
void* CSSymbolicatorCreateWithTask(void);
CSSymbolicatorRef CSSymbolicatorCreateWithTaskFlagsAndNotification(task_t task, uint32_t flags, void (^handler)(uint32_t notification_type, CSNotificationData data));
void* CSSymbolicatorCreateWithTaskPidFlagsAndNotification(void);
void* CSSymbolicatorCreateWithURLAndArchitecture(void);
void* CSSymbolicatorCreateWithURLArchitectureFlagsAndNotification(void);
void* CSSymbolicatorFixupFrames(void);
void* CSSymbolicatorForceFullSymbolExtraction(void);
void* CSSymbolicatorForeachRegionAtTime(void);
void* CSSymbolicatorForeachRegionWithNameAtTime(void);
void* CSSymbolicatorForeachSectionAtTime(void);
void* CSSymbolicatorForeachSegmentAtTime(void);
void* CSSymbolicatorForeachSharedCache(void);
void* CSSymbolicatorForeachSharedCacheSymbolicatorWithFlagsAndNotification(void);
void* CSSymbolicatorForeachSourceInfoAtTime(void);
void* CSSymbolicatorForeachSymbolAtTime(void);
int CSSymbolicatorForeachSymbolOwnerAtTime(CSSymbolicatorRef symbolicator, int time, void (^handler)(CSSymbolOwnerRef t));
void* CSSymbolicatorForeachSymbolOwnerWithCFUUIDBytesAtTime(void);
int CSSymbolicatorForeachSymbolOwnerWithFlagsAtTime(CSSymbolicatorRef symbolicator, int flags, int time, void (^handler)(CSSymbolOwnerRef t));
int CSSymbolicatorForeachSymbolOwnerWithNameAtTime(CSSymbolicatorRef symbolicator, const char* name, int time, void (^handler)(CSSymbolOwnerRef t));
int CSSymbolicatorForeachSymbolOwnerWithPathAtTime(CSSymbolicatorRef symbolicator, const char* path, int time, void (^handler)(CSSymbolOwnerRef t));
void* CSSymbolicatorForeachSymbolOwnerWithUUIDAtTime(void);
void CSSymbolicatorForeachSymbolWithMangledNameAtTime(CSSymbolicatorRef symbolicator, const char* name, int time, void (^handler)(CSSymbolRef t));
void CSSymbolicatorForeachSymbolWithNameAtTime(CSSymbolicatorRef symbolicator, const char* name, int time, void (^handler)(CSSymbolRef t));
void* CSSymbolicatorForeachSymbolicatorWithPath(void);
void* CSSymbolicatorForeachSymbolicatorWithPathFlagsAndNotification(void);
void* CSSymbolicatorForeachSymbolicatorWithURL(void);
void* CSSymbolicatorForeachSymbolicatorWithURLFlagsAndNotification(void);
void* CSSymbolicatorGetAOutSymbolOwner(int arg0);
int CSSymbolicatorGetArchitecture(CSSymbolicatorRef symbolicator);
void* CSSymbolicatorGetFlagsForDebugMapOnlyData(void);
void* CSSymbolicatorGetFlagsForDsymOnlyData(void);
void* CSSymbolicatorGetFlagsForDwarfOnlyData(void);
void* CSSymbolicatorGetFlagsForFunctionStartsOnlyData(void);
void* CSSymbolicatorGetFlagsForNListOnlyData(void);
void* CSSymbolicatorGetFlagsForNoSymbolOrSourceInfoData(void);
pid_t CSSymbolicatorGetPid(CSSymbolicatorRef symbolicator);
void* CSSymbolicatorGetRegionCountAtTime(void);
void* CSSymbolicatorGetRegionWithAddressAtTime(void);
void* CSSymbolicatorGetRegionWithNameAtTime(void);
void* CSSymbolicatorGetSectionWithAddressAtTime(void);
void* CSSymbolicatorGetSegmentWithAddressAtTime(void);
void* CSSymbolicatorGetSharedCacheSlide(void);
void* CSSymbolicatorGetSharedCacheUUID(void);
void* CSSymbolicatorGetSourceInfoCountAtTime(void);
CSSourceInfoRef CSSymbolicatorGetSourceInfoWithAddressAtTime(CSSymbolicatorRef, mach_vm_address_t, time_t);
void* CSSymbolicatorGetSymbolCountAtTime(void);
void* CSSymbolicatorGetSymbolOwner(void);
void* CSSymbolicatorGetSymbolOwnerCountAtTime(void);
CSSymbolOwnerRef CSSymbolicatorGetSymbolOwnerWithAddressAtTime(CSSymbolicatorRef symbolicator, mach_vm_address_t address, int time);
void* CSSymbolicatorGetSymbolOwnerWithCFUUIDBytesAtTime(void);
void* CSSymbolicatorGetSymbolOwnerWithNameAtTime(void);
CSSymbolOwnerRef CSSymbolicatorGetSymbolOwnerWithUUIDAtTime(CSSymbolicatorRef symbolicator, CFUUIDRef uuid, int time);
CSSymbolRef CSSymbolicatorGetSymbolWithAddressAtTime(CSSymbolicatorRef symbolicator, mach_vm_address_t addr, time_t time);
void* CSSymbolicatorGetSymbolWithMangledNameAtTime(void);
void* CSSymbolicatorGetSymbolWithMangledNameFromSymbolOwnerWithNameAtTime(void);
CSSymbolRef CSSymbolicatorGetSymbolWithNameAtTime(CSSymbolicatorRef symbolicator, char* name, void* unknown);
void* CSSymbolicatorGetSymbolWithNameFromSymbolOwnerWithNameAtTime(void);
task_t CSSymbolicatorGetTask(CSSymbolicatorRef symbolicator);
Boolean CSSymbolicatorIsKernelSymbolicator(CSSymbolicatorRef);
void* CSSymbolicatorIsTaskTranslated(void);
void* CSSymbolicatorIsTaskValid(void);
void* CSSymbolicatorResymbolicate(void);
void* CSSymbolicatorResymbolicateFail(void);
void* CSSymbolicatorResymbolicateFromDebugSymbolsInfo(void);
void* CSSymbolicatorSetForceGlobalSafeMachVMReads(void);
void CSSymbolicatorSubscribeToTaskMainNotification(CSSymbolicatorRef symbolicator);
void* CSTotalBytesMapped(void);
void* CSUUIDCFUUIDBytesToPath(void);
void* CSUUIDCFUUIDBytesToString(void);
void* CSUUIDStringToCFUUIDBytes(void);
void* clear_mapped_memory(void);
void* create_mapped_memory_cache_for_task(void);
void* create_sampling_context_for_task(void);
void* create_sampling_context_for_task_and_pid(void);
void* create_sampling_context_for_task_pid_symbolicator(void);
void* demangle(void);
void* destroy_mapped_memory_cache(void);
void* destroy_sampling_context(void);
void* dispatch_queue_name_for_serial_number(void);
void* find_node(void);
void* get_remote_thread_dispatch_queue(void);
void* map_new_node(void);
void* mapped_memory_read(void);
void* mapped_memory_read_pointer(void);
void* next_node(void);
void* sample_remote_thread(void);
void* sample_remote_thread_with_dispatch_queue(void);
void* sampling_context_clear_cache(void);
void* task_is_64bit(void);
void* thread_name_for_thread_port(void);

#endif
