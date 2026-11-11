/*
 This file is part of Darling.

 Copyright (C) 2025-2026 Darling Developers

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

#ifndef _CORESERVICES_LSSHAREDFILELIST_H_
#define _CORESERVICES_LSSHAREDFILELIST_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/MacTypes.h>
#include <CarbonCore/MacErrors.h>
#include <CoreServices/FileManager.h>
#include <LaunchServices/IconsCore.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CF_BRIDGED_MUTABLE_TYPE
#if __has_attribute(objc_bridge_mutable)
#define CF_BRIDGED_MUTABLE_TYPE(type) __attribute__((objc_bridge_mutable(type)))
#else
#define CF_BRIDGED_MUTABLE_TYPE(type)
#endif
#endif

typedef struct CF_BRIDGED_MUTABLE_TYPE(id) OpaqueLSSharedFileListRef *LSSharedFileListRef;
typedef struct CF_BRIDGED_MUTABLE_TYPE(id) OpaqueLSSharedFileListItemRef *LSSharedFileListItemRef;

extern LSSharedFileListItemRef kLSSharedFileListItemLast;
extern LSSharedFileListItemRef kLSSharedFileListItemBeforeFirst;

/* List type constants */
extern const CFStringRef kLSSharedFileListFavoriteVolumes;
extern const CFStringRef kLSSharedFileListFavoriteItems;
extern const CFStringRef kLSSharedFileListRecentApplicationItems;
extern const CFStringRef kLSSharedFileListRecentDocumentItems;
extern const CFStringRef kLSSharedFileListRecentServerItems;
extern const CFStringRef kLSSharedFileListSessionLoginItems;
extern const CFStringRef kLSSharedFileListGlobalLoginItems;

/* Property keys */
extern const CFStringRef kLSSharedFileListRecentItemsMaxAmount;
extern const CFStringRef kLSSharedFileListVolumesComputerVisible;
extern const CFStringRef kLSSharedFileListVolumesIDiskVisible;
extern const CFStringRef kLSSharedFileListVolumesNetworkVisible;
extern const CFStringRef kLSSharedFileListItemHidden;
extern const CFStringRef kLSSharedFileListLoginItemHidden;

/* Flags */
enum {
    kLSSharedFileListNoUserInteraction = 1 << 0,
    kLSSharedFileListDoNotMountVolumes = 1 << 1
};

/*
 * Lists and their items are Core Foundation objects that live in memory only: nothing is
 * stored on disk, so items inserted by one process (or by a later run) are not seen by others.
 */
CFTypeID LSSharedFileListGetTypeID(void);
CFTypeID LSSharedFileListItemGetTypeID(void);

LSSharedFileListRef LSSharedFileListCreate(
    CFAllocatorRef inAllocator,
    CFStringRef inListType,
    CFTypeRef listOptions);

CFArrayRef LSSharedFileListCopySnapshot(
    LSSharedFileListRef inList,
    UInt32 *outSnapshotSeed);

OSStatus LSSharedFileListItemResolve(
    LSSharedFileListItemRef inItem,
    UInt32 inFlags,
    CFURLRef *outURL,
    FSRef *outRef);

LSSharedFileListItemRef LSSharedFileListInsertItemURL(
    LSSharedFileListRef inList,
    LSSharedFileListItemRef insertAfterThisItem,
    CFStringRef inDisplayName,
    IconRef inIconRef,
    CFURLRef inURL,
    CFDictionaryRef inPropertiesToSet,
    CFArrayRef inPropertiesToClear);

OSStatus LSSharedFileListItemRemove(
    LSSharedFileListRef inList,
    LSSharedFileListItemRef inItem);

OSStatus LSSharedFileListRemoveAllItems(
    LSSharedFileListRef inList);

UInt32 LSSharedFileListGetSeedValue(
    LSSharedFileListRef inList);

UInt32 LSSharedFileListItemGetID(
    LSSharedFileListItemRef inItem);

CFStringRef LSSharedFileListItemCopyDisplayName(
    LSSharedFileListItemRef inItem);

CFTypeRef LSSharedFileListCopyProperty(
    LSSharedFileListRef inList,
    CFStringRef inPropertyName);

OSStatus LSSharedFileListSetProperty(
    LSSharedFileListRef inList,
    CFStringRef inPropertyName,
    CFTypeRef inPropertyData);

CFTypeRef LSSharedFileListItemCopyProperty(
    LSSharedFileListItemRef inItem,
    CFStringRef inPropertyName);

OSStatus LSSharedFileListItemSetProperty(
    LSSharedFileListItemRef inItem,
    CFStringRef inPropertyName,
    CFTypeRef inPropertyData);

#ifdef __cplusplus
}
#endif

#endif
