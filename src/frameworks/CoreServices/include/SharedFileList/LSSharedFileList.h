/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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

typedef struct OpaqueLSSharedFileListRef *LSSharedFileListRef;
typedef struct OpaqueLSSharedFileListItemRef *LSSharedFileListItemRef;
typedef struct OpaqueIconRef *IconRef;

#ifdef __cplusplus
extern "C" {
#endif

extern LSSharedFileListRef LSSharedFileListCreate(CFAllocatorRef inAllocator, CFStringRef inListType, CFTypeRef listOptions);
extern CFArrayRef LSSharedFileListCopySnapshot(LSSharedFileListRef inList, UInt32 *outSnapshotSeed);
extern LSSharedFileListItemRef LSSharedFileListInsertItemURL(
	LSSharedFileListRef inList,
	LSSharedFileListItemRef insertAfterThisItem,
	CFStringRef inDisplayName,
	IconRef inIconRef,
	CFURLRef inURL,
	CFDictionaryRef inPropertiesToSet,
	CFArrayRef inPropertiesToClear);
extern CFTypeRef LSSharedFileListItemCopyProperty(LSSharedFileListItemRef inItem, CFStringRef inCFStringKey);
extern CFURLRef LSSharedFileListItemCopyResolvedURL(LSSharedFileListItemRef inItem, UInt32 inFlags, CFErrorRef *outError);
extern OSStatus LSSharedFileListItemRemove(LSSharedFileListRef inList, LSSharedFileListItemRef inItem);

#ifdef __cplusplus
}
#endif

extern LSSharedFileListItemRef kLSSharedFileListItemLast;

#endif
