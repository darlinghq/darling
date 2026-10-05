/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

 Darling is free software; you can redistribute it and/or modify
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

#include <SharedFileList/LSSharedFileList.h>
#include <CoreFoundation/CFRuntime.h>
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

struct __LSSharedFileList {
	CFRuntimeBase _base;
	pthread_mutex_t lock;
	CFMutableArrayRef items;
	CFStringRef listType;
	CFMutableDictionaryRef properties;
	UInt32 seed;
};

struct __LSSharedFileListItem {
	CFRuntimeBase _base;
	CFURLRef url;
	CFStringRef displayName;
	CFMutableDictionaryRef properties;
	UInt32 id;
};

static CFTypeID __kLSSharedFileListTypeID = _kCFRuntimeNotATypeID;
static CFTypeID __kLSSharedFileListItemTypeID = _kCFRuntimeNotATypeID;

static void __LSSharedFileListDeallocate(CFTypeRef cf) {
	struct __LSSharedFileList *list = (struct __LSSharedFileList *)cf;
	pthread_mutex_lock(&list->lock);
	if (list->items) {
		CFRelease(list->items);
		list->items = NULL;
	}
	if (list->listType) {
		CFRelease(list->listType);
		list->listType = NULL;
	}
	if (list->properties) {
		CFRelease(list->properties);
		list->properties = NULL;
	}
	pthread_mutex_unlock(&list->lock);
	pthread_mutex_destroy(&list->lock);
}

static const CFRuntimeClass __LSSharedFileListClass = {
	_kCFRuntimeScannedObject,
	"LSSharedFileList",
	NULL,
	NULL,
	__LSSharedFileListDeallocate,
	NULL,
	NULL,
	NULL,
	NULL,
};

static void __LSSharedFileListItemDeallocate(CFTypeRef cf) {
	struct __LSSharedFileListItem *item = (struct __LSSharedFileListItem *)cf;
	if (item->url) {
		CFRelease(item->url);
		item->url = NULL;
	}
	if (item->displayName) {
		CFRelease(item->displayName);
		item->displayName = NULL;
	}
	if (item->properties) {
		CFRelease(item->properties);
		item->properties = NULL;
	}
}

static const CFRuntimeClass __LSSharedFileListItemClass = {
	_kCFRuntimeScannedObject,
	"LSSharedFileListItem",
	NULL,
	NULL,
	__LSSharedFileListItemDeallocate,
	NULL,
	NULL,
	NULL,
	NULL,
};

CFTypeID LSSharedFileListGetTypeID(void) {
	static dispatch_once_t initOnce;
	dispatch_once(&initOnce, ^{
		__kLSSharedFileListTypeID = _CFRuntimeRegisterClass(&__LSSharedFileListClass);
	});
	return __kLSSharedFileListTypeID;
}

CFTypeID LSSharedFileListItemGetTypeID(void) {
	static dispatch_once_t initOnce;
	dispatch_once(&initOnce, ^{
		__kLSSharedFileListItemTypeID = _CFRuntimeRegisterClass(&__LSSharedFileListItemClass);
	});
	return __kLSSharedFileListItemTypeID;
}

LSSharedFileListRef LSSharedFileListCreate(
	CFAllocatorRef inAllocator,
	CFStringRef inListType,
	CFTypeRef listOptions)
{
	(void)listOptions;

	struct __LSSharedFileList *list = (struct __LSSharedFileList *)_CFRuntimeCreateInstance(
		inAllocator, LSSharedFileListGetTypeID(),
		sizeof(struct __LSSharedFileList) - sizeof(CFRuntimeBase), NULL);
	if (!list) {
		return NULL;
	}

	pthread_mutex_init(&list->lock, NULL);
	list->items = CFArrayCreateMutable(inAllocator, 0, &kCFTypeArrayCallBacks);
	list->listType = inListType ? CFStringCreateCopy(inAllocator, inListType) : NULL;
	list->properties = CFDictionaryCreateMutable(inAllocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	list->seed = 1;

	return (LSSharedFileListRef)list;
}

CFArrayRef LSSharedFileListCopySnapshot(LSSharedFileListRef inList, UInt32 *outSnapshotSeed) {
	if (!inList) {
		return NULL;
	}

	struct __LSSharedFileList *list = (struct __LSSharedFileList *)inList;
	CFArrayRef snapshot = NULL;

	pthread_mutex_lock(&list->lock);
	if (outSnapshotSeed) {
		*outSnapshotSeed = list->seed;
	}

	if (list->items) {
		snapshot = CFArrayCreateCopy(CFGetAllocator(inList), list->items);
	} else {
		snapshot = CFArrayCreate(CFGetAllocator(inList), NULL, 0, &kCFTypeArrayCallBacks);
	}
	pthread_mutex_unlock(&list->lock);

	return snapshot;
}

OSStatus LSSharedFileListItemResolve(
	LSSharedFileListItemRef inItem,
	UInt32 inFlags,
	CFURLRef *outURL,
	FSRef *outRef)
{
	(void)inFlags;

	if (outURL) {
		*outURL = NULL;
	}

	if (outRef) {
		memset(outRef, 0, sizeof(FSRef));
	}

	if (!inItem) {
		return paramErr;
	}

	struct __LSSharedFileListItem *item = (struct __LSSharedFileListItem *)inItem;
	if (outURL) {
		if (item->url) {
			*outURL = (CFURLRef)CFRetain(item->url);
		} else {
			*outURL = NULL;
		}
	}
	return noErr;
}

LSSharedFileListItemRef LSSharedFileListInsertItemURL(
	LSSharedFileListRef inList,
	LSSharedFileListItemRef insertAfterThisItem,
	CFStringRef inDisplayName,
	IconRef inIconRef,
	CFURLRef inURL,
	CFDictionaryRef inPropertiesToSet,
	CFArrayRef inPropertiesToClear)
{
	(void)inIconRef;

	if (!inList || !inURL) {
		return NULL;
	}

	struct __LSSharedFileList *list = (struct __LSSharedFileList *)inList;
	CFAllocatorRef alloc = CFGetAllocator(inList);

	struct __LSSharedFileListItem *item = (struct __LSSharedFileListItem *)_CFRuntimeCreateInstance(
		alloc, LSSharedFileListItemGetTypeID(),
		sizeof(struct __LSSharedFileListItem) - sizeof(CFRuntimeBase), NULL);
	if (!item) {
		return NULL;
	}

	item->url = (CFURLRef)CFRetain(inURL);
	item->displayName = inDisplayName ? CFStringCreateCopy(alloc, inDisplayName) : NULL;
	item->properties = CFDictionaryCreateMutable(alloc, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	static atomic_uint_fast32_t s_next_id = 1;
	item->id = atomic_fetch_add(&s_next_id, 1);

	if (inPropertiesToSet && item->properties) {
		CFIndex count = CFDictionaryGetCount(inPropertiesToSet);
		if (count > 0) {
			const void **keys = (const void **)malloc(count * sizeof(void *));
			const void **values = (const void **)malloc(count * sizeof(void *));
			if (keys && values) {
				CFDictionaryGetKeysAndValues(inPropertiesToSet, keys, values);
				for (CFIndex i = 0; i < count; i++) {
					CFDictionarySetValue(item->properties, keys[i], values[i]);
				}
			}
			free(keys);
			free(values);
		}
	}

	if (inPropertiesToClear && item->properties) {
		CFIndex count = CFArrayGetCount(inPropertiesToClear);
		for (CFIndex i = 0; i < count; i++) {
			const void *key = CFArrayGetValueAtIndex(inPropertiesToClear, i);
			if (key) {
				CFDictionaryRemoveValue(item->properties, key);
			}
		}
	}

	pthread_mutex_lock(&list->lock);
	if (list->items) {
		CFIndex insertIdx = CFArrayGetCount(list->items);
		if (insertAfterThisItem == kLSSharedFileListItemBeforeFirst) {
			insertIdx = 0;
		} else if (insertAfterThisItem && insertAfterThisItem != kLSSharedFileListItemLast) {
			CFIndex count = CFArrayGetCount(list->items);
			for (CFIndex i = 0; i < count; i++) {
				if (CFArrayGetValueAtIndex(list->items, i) == (const void *)insertAfterThisItem) {
					insertIdx = i + 1;
					break;
				}
			}
		}
		CFArrayInsertValueAtIndex(list->items, insertIdx, item);
		list->seed++;
	}
	pthread_mutex_unlock(&list->lock);

	return (LSSharedFileListItemRef)item;
}

OSStatus LSSharedFileListItemRemove(LSSharedFileListRef inList, LSSharedFileListItemRef inItem) {
	if (!inList || !inItem) {
		return paramErr;
	}

	struct __LSSharedFileList *list = (struct __LSSharedFileList *)inList;
	pthread_mutex_lock(&list->lock);
	if (!list->items) {
		pthread_mutex_unlock(&list->lock);
		return noErr;
	}

	CFIndex count = CFArrayGetCount(list->items);
	for (CFIndex i = 0; i < count; i++) {
		if (CFArrayGetValueAtIndex(list->items, i) == (const void *)inItem) {
			CFArrayRemoveValueAtIndex(list->items, i);
			list->seed++;
			break;
		}
	}
	pthread_mutex_unlock(&list->lock);
	return noErr;
}

OSStatus LSSharedFileListRemoveAllItems(LSSharedFileListRef inList) {
	if (!inList) {
		return paramErr;
	}

	struct __LSSharedFileList *list = (struct __LSSharedFileList *)inList;
	pthread_mutex_lock(&list->lock);
	if (list->items) {
		CFArrayRemoveAllValues(list->items);
		list->seed++;
	}
	pthread_mutex_unlock(&list->lock);
	return noErr;
}

UInt32 LSSharedFileListGetSeedValue(LSSharedFileListRef inList) {
	if (!inList) {
		return 0;
	}
	struct __LSSharedFileList *list = (struct __LSSharedFileList *)inList;
	pthread_mutex_lock(&list->lock);
	UInt32 seed = list->seed;
	pthread_mutex_unlock(&list->lock);
	return seed;
}

UInt32 LSSharedFileListItemGetID(LSSharedFileListItemRef inItem) {
	if (!inItem) {
		return 0;
	}
	return ((struct __LSSharedFileListItem *)inItem)->id;
}

CFStringRef LSSharedFileListItemCopyDisplayName(LSSharedFileListItemRef inItem) {
	if (!inItem) {
		return NULL;
	}
	struct __LSSharedFileListItem *item = (struct __LSSharedFileListItem *)inItem;
	if (item->displayName) {
		return CFStringCreateCopy(CFGetAllocator(inItem), item->displayName);
	}
	return NULL;
}

CFTypeRef LSSharedFileListCopyProperty(LSSharedFileListRef inList, CFStringRef inPropertyName) {
	if (!inList || !inPropertyName) {
		return NULL;
	}
	struct __LSSharedFileList *list = (struct __LSSharedFileList *)inList;
	CFTypeRef val = NULL;
	pthread_mutex_lock(&list->lock);
	if (list->properties) {
		CFTypeRef orig = CFDictionaryGetValue(list->properties, inPropertyName);
		if (orig) {
			val = CFRetain(orig);
		}
	}
	pthread_mutex_unlock(&list->lock);
	return val;
}

OSStatus LSSharedFileListSetProperty(LSSharedFileListRef inList, CFStringRef inPropertyName, CFTypeRef inPropertyData) {
	if (!inList || !inPropertyName) {
		return paramErr;
	}
	struct __LSSharedFileList *list = (struct __LSSharedFileList *)inList;
	pthread_mutex_lock(&list->lock);
	if (!list->properties) {
		pthread_mutex_unlock(&list->lock);
		return memFullErr;
	}
	if (inPropertyData) {
		CFDictionarySetValue(list->properties, inPropertyName, inPropertyData);
	} else {
		CFDictionaryRemoveValue(list->properties, inPropertyName);
	}
	pthread_mutex_unlock(&list->lock);
	return noErr;
}

CFTypeRef LSSharedFileListItemCopyProperty(LSSharedFileListItemRef inItem, CFStringRef inPropertyName) {
	if (!inItem || !inPropertyName) {
		return NULL;
	}
	struct __LSSharedFileListItem *item = (struct __LSSharedFileListItem *)inItem;
	if (item->properties) {
		CFTypeRef val = CFDictionaryGetValue(item->properties, inPropertyName);
		if (val) {
			return CFRetain(val);
		}
	}
	return NULL;
}

OSStatus LSSharedFileListItemSetProperty(LSSharedFileListItemRef inItem, CFStringRef inPropertyName, CFTypeRef inPropertyData) {
	if (!inItem || !inPropertyName) {
		return paramErr;
	}
	struct __LSSharedFileListItem *item = (struct __LSSharedFileListItem *)inItem;
	if (!item->properties) {
		return memFullErr;
	}
	if (inPropertyData) {
		CFDictionarySetValue(item->properties, inPropertyName, inPropertyData);
	} else {
		CFDictionaryRemoveValue(item->properties, inPropertyName);
	}
	return noErr;
}
