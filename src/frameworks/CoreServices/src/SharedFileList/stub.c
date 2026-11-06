/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

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

#include <SharedFileList/SharedFileList.h>
#include <CarbonCore/MacErrors.h>
#include <stdio.h>
#include <stdlib.h>

static int verbose = 0;

__attribute__((constructor)) static void initme(void) {
	verbose = getenv("STUB_VERBOSE") != NULL;
}

LSSharedFileListRef LSSharedFileListCreate(CFAllocatorRef inAllocator, CFStringRef inListType, CFTypeRef listOptions)
{
	if (verbose)
		puts("STUB: LSSharedFileListCreate called");
	return NULL;
}

CFArrayRef LSSharedFileListCopySnapshot(LSSharedFileListRef inList, UInt32 *outSnapshotSeed)
{
	if (verbose)
		puts("STUB: LSSharedFileListCopySnapshot called");
	return NULL;
}

LSSharedFileListItemRef LSSharedFileListInsertItemURL(
	LSSharedFileListRef inList,
	LSSharedFileListItemRef insertAfterThisItem,
	CFStringRef inDisplayName,
	IconRef inIconRef,
	CFURLRef inURL,
	CFDictionaryRef inPropertiesToSet,
	CFArrayRef inPropertiesToClear
) {
	if (verbose)
		puts("STUB: LSSharedFileListInsertItemURL called");
	return NULL;
}

CFTypeRef LSSharedFileListItemCopyProperty(LSSharedFileListItemRef inItem, CFStringRef inCFStringKey)
{
	if (verbose)
		puts("STUB: LSSharedFileListItemCopyProperty called");
	return NULL;
}

CFURLRef LSSharedFileListItemCopyResolvedURL(LSSharedFileListItemRef inItem, UInt32 inFlags, CFErrorRef *outError)
{
	if (verbose)
		puts("STUB: LSSharedFileListItemCopyResolvedURL called");
	return NULL;
}

OSStatus LSSharedFileListItemRemove(LSSharedFileListRef inList, LSSharedFileListItemRef inItem)
{
	if (verbose)
		puts("STUB: LSSharedFileListItemRemove called");
	return paramErr;
}
