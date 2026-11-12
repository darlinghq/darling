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
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

// What identifies one version of the list's file, to notice changes made by other processes.
struct __LSSharedFileListFileStamp {
	Boolean valid;
	dev_t device;
	ino_t inode;
	off_t size;
	struct timespec modified;
};

struct __LSSharedFileList {
	CFRuntimeBase _base;
	pthread_mutex_t lock;
	CFMutableArrayRef items;
	CFStringRef listType;
	CFMutableDictionaryRef properties;
	UInt32 seed;
	char path[PATH_MAX]; // the property list the list is kept in, empty when there is no home
	struct __LSSharedFileListFileStamp stamp;
};

struct __LSSharedFileListItem {
	CFRuntimeBase _base;
	CFURLRef url;
	CFStringRef displayName;
	CFMutableDictionaryRef properties;
	UInt32 id;
	struct __LSSharedFileList *list; // not retained; cleared when the item leaves the list
};

static CFTypeID __kLSSharedFileListTypeID = _kCFRuntimeNotATypeID;
static CFTypeID __kLSSharedFileListItemTypeID = _kCFRuntimeNotATypeID;

static void __LSSharedFileListDeallocate(CFTypeRef cf) {
	struct __LSSharedFileList *list = (struct __LSSharedFileList *)cf;
	if (list->items) {
		CFIndex count = CFArrayGetCount(list->items);
		for (CFIndex i = 0; i < count; i++) {
			((struct __LSSharedFileListItem *)CFArrayGetValueAtIndex(list->items, i))->list = NULL;
		}
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

// ---- Persistence ----
//
// A list is kept in a property list of its own, ~/Library/Application Support/
// com.apple.sharedfilelist/<list type>.plist (macOS keeps lists in a service of its own and its
// storage cannot be observed; this file is Darling's format). Every change is written at once
// and the file is read again when another process has replaced it, so lists survive the
// process and are shared by processes. Property values that are not property-list types are
// not written.

static const char *kSharedFileListDirectory = "Library/Application Support/com.apple.sharedfilelist";

static Boolean __LSSharedFileListStat(const char *path, struct __LSSharedFileListFileStamp *stamp) {
	struct stat st;
	memset(stamp, 0, sizeof(*stamp));
	if (stat(path, &st) != 0) {
		return false;
	}
	stamp->valid = true;
	stamp->device = st.st_dev;
	stamp->inode = st.st_ino;
	stamp->size = st.st_size;
	stamp->modified = st.st_mtimespec;
	return true;
}

static Boolean __LSSharedFileListStampsEqual(const struct __LSSharedFileListFileStamp *a, const struct __LSSharedFileListFileStamp *b) {
	return a->valid == b->valid && a->device == b->device && a->inode == b->inode && a->size == b->size
		&& a->modified.tv_sec == b->modified.tv_sec && a->modified.tv_nsec == b->modified.tv_nsec;
}

// Builds the file name from the list type; path separators are replaced so the type stays one name.
static void __LSSharedFileListComputePath(struct __LSSharedFileList *list) {
	list->path[0] = '\0';

	const char *home = getenv("HOME");
	struct passwd pwd, *result = NULL;
	char buffer[1024];
	if ((!home || !*home) && getpwuid_r(getuid(), &pwd, buffer, sizeof(buffer), &result) == 0 && result) {
		home = pwd.pw_dir;
	}
	if (!home || !*home) {
		return;
	}

	char name[256] = "unnamed";
	if (list->listType) {
		char type[256];
		if (CFStringGetCString(list->listType, type, sizeof(type), kCFStringEncodingUTF8) && type[0]) {
			for (char *c = type; *c; c++) {
				if (*c == '/') {
					*c = '_';
				}
			}
			if (strcmp(type, ".") != 0 && strcmp(type, "..") != 0) {
				strlcpy(name, type, sizeof(name));
			}
		}
	}
	snprintf(list->path, sizeof(list->path), "%s/%s/%s.plist", home, kSharedFileListDirectory, name);
}

static void __LSSharedFileListMakeDirectory(const char *path) {
	char directory[PATH_MAX];
	strlcpy(directory, path, sizeof(directory));
	char *slash = strrchr(directory, '/');
	if (!slash) {
		return;
	}
	*slash = '\0';
	for (char *c = directory + 1; *c; c++) {
		if (*c == '/') {
			*c = '\0';
			mkdir(directory, 0755);
			*c = '/';
		}
	}
	mkdir(directory, 0755);
}

// Keeps other processes' read-modify-write sequences on this list apart. Returns the descriptor
// to pass to __LSSharedFileListUnlockFile, or -1 when there is nothing to lock.
static int __LSSharedFileListLockFile(struct __LSSharedFileList *list) {
	if (!list->path[0]) {
		return -1;
	}
	char lockPath[PATH_MAX];
	snprintf(lockPath, sizeof(lockPath), "%s.lock", list->path);
	__LSSharedFileListMakeDirectory(list->path);
	int fd = open(lockPath, O_RDWR | O_CREAT | O_CLOEXEC, 0600);
	if (fd >= 0 && flock(fd, LOCK_EX) != 0) {
		close(fd);
		fd = -1;
	}
	return fd;
}

static void __LSSharedFileListUnlockFile(int fd) {
	if (fd >= 0) {
		flock(fd, LOCK_UN);
		close(fd);
	}
}

static CFStringRef __LSSharedFileListCopyURLString(CFURLRef url) {
	CFStringRef string = CFURLGetString(url);
	return string ? CFStringCreateCopy(kCFAllocatorDefault, string) : NULL;
}

// A copy of `properties` without the values that cannot be written to a property list.
static CFDictionaryRef __LSSharedFileListCopyStorableProperties(CFDictionaryRef properties) {
	CFMutableDictionaryRef result = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFIndex count = properties ? CFDictionaryGetCount(properties) : 0;
	if (count > 0 && result) {
		const void **keys = (const void **)malloc(count * sizeof(void *));
		const void **values = (const void **)malloc(count * sizeof(void *));
		if (keys && values) {
			CFDictionaryGetKeysAndValues(properties, keys, values);
			for (CFIndex i = 0; i < count; i++) {
				if (CFGetTypeID(keys[i]) == CFStringGetTypeID() && CFPropertyListIsValid((CFPropertyListRef)values[i], kCFPropertyListXMLFormat_v1_0)) {
					CFDictionarySetValue(result, keys[i], values[i]);
				}
			}
		}
		free(keys);
		free(values);
	}
	return result;
}

// Writes the list to its file (the lock of the list is held). Failures keep the list in memory only.
static void __LSSharedFileListSaveLocked(struct __LSSharedFileList *list) {
	if (!list->path[0]) {
		return;
	}

	CFMutableArrayRef entries = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	CFIndex count = CFArrayGetCount(list->items);
	for (CFIndex i = 0; i < count; i++) {
		struct __LSSharedFileListItem *item = (struct __LSSharedFileListItem *)CFArrayGetValueAtIndex(list->items, i);
		CFMutableDictionaryRef entry = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		SInt64 identifier = item->id;
		CFNumberRef number = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &identifier);
		CFDictionarySetValue(entry, CFSTR("id"), number);
		CFRelease(number);
		if (item->displayName) {
			CFDictionarySetValue(entry, CFSTR("name"), item->displayName);
		}
		CFStringRef url = item->url ? __LSSharedFileListCopyURLString(item->url) : NULL;
		if (url) {
			CFDictionarySetValue(entry, CFSTR("url"), url);
			CFRelease(url);
		}
		CFDictionaryRef properties = __LSSharedFileListCopyStorableProperties(item->properties);
		CFDictionarySetValue(entry, CFSTR("properties"), properties);
		CFRelease(properties);
		CFArrayAppendValue(entries, entry);
		CFRelease(entry);
	}

	CFMutableDictionaryRef root = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	SInt64 seed = list->seed;
	CFNumberRef seedNumber = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &seed);
	CFDictionarySetValue(root, CFSTR("seed"), seedNumber);
	CFRelease(seedNumber);
	CFDictionarySetValue(root, CFSTR("items"), entries);
	CFRelease(entries);
	CFDictionaryRef properties = __LSSharedFileListCopyStorableProperties(list->properties);
	CFDictionarySetValue(root, CFSTR("properties"), properties);
	CFRelease(properties);

	CFDataRef data = CFPropertyListCreateData(kCFAllocatorDefault, root, kCFPropertyListXMLFormat_v1_0, 0, NULL);
	CFRelease(root);
	if (!data) {
		return;
	}

	__LSSharedFileListMakeDirectory(list->path);
	char temporary[PATH_MAX];
	snprintf(temporary, sizeof(temporary), "%s.%d.tmp", list->path, (int)getpid());
	int fd = open(temporary, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
	Boolean written = false;
	if (fd >= 0) {
		const UInt8 *bytes = CFDataGetBytePtr(data);
		CFIndex remaining = CFDataGetLength(data);
		written = true;
		while (remaining > 0) {
			ssize_t n = write(fd, bytes, remaining);
			if (n < 0 && errno == EINTR) {
				continue;
			}
			if (n <= 0) {
				written = false;
				break;
			}
			bytes += n;
			remaining -= n;
		}
		close(fd);
	}
	CFRelease(data);

	if (written && rename(temporary, list->path) == 0) {
		__LSSharedFileListStat(list->path, &list->stamp);
	} else {
		unlink(temporary);
	}
}

static LSSharedFileListItemRef __LSSharedFileListFindItemLocked(struct __LSSharedFileList *list, LSSharedFileListItemRef item, CFIndex *outIndex) {
	if (!item || !list->items) {
		return NULL;
	}
	UInt32 identifier = ((struct __LSSharedFileListItem *)item)->id;
	CFIndex count = CFArrayGetCount(list->items);
	for (CFIndex i = 0; i < count; i++) {
		struct __LSSharedFileListItem *candidate = (struct __LSSharedFileListItem *)CFArrayGetValueAtIndex(list->items, i);
		if ((LSSharedFileListItemRef)candidate == item || candidate->id == identifier) {
			if (outIndex) {
				*outIndex = i;
			}
			return (LSSharedFileListItemRef)candidate;
		}
	}
	return NULL;
}

static struct __LSSharedFileListItem *__LSSharedFileListCreateItem(struct __LSSharedFileList *list, CFAllocatorRef alloc) {
	struct __LSSharedFileListItem *item = (struct __LSSharedFileListItem *)_CFRuntimeCreateInstance(
		alloc, LSSharedFileListItemGetTypeID(),
		sizeof(struct __LSSharedFileListItem) - sizeof(CFRuntimeBase), NULL);
	if (item) {
		item->properties = CFDictionaryCreateMutable(alloc, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		item->list = list;
	}
	return item;
}

// Reads the file again when it was replaced since this list wrote or read it (the lock of the
// list is held). Items that are still in the file keep their objects, so references handed out
// earlier stay valid; the seed is the file's, so it changes with every change made anywhere.
static void __LSSharedFileListSyncLocked(struct __LSSharedFileList *list) {
	if (!list->path[0]) {
		return;
	}

	struct __LSSharedFileListFileStamp current;
	__LSSharedFileListStat(list->path, &current);
	if (__LSSharedFileListStampsEqual(&current, &list->stamp)) {
		return;
	}
	list->stamp = current;

	CFAllocatorRef alloc = CFGetAllocator((CFTypeRef)list);
	CFDictionaryRef root = NULL;
	if (current.valid) {
		int fd = open(list->path, O_RDONLY | O_CLOEXEC);
		if (fd >= 0) {
			CFMutableDataRef data = CFDataCreateMutable(alloc, 0);
			UInt8 buffer[4096];
			ssize_t n;
			while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
				CFDataAppendBytes(data, buffer, n);
			}
			close(fd);
			CFPropertyListRef plist = CFPropertyListCreateWithData(alloc, data, kCFPropertyListImmutable, NULL, NULL);
			CFRelease(data);
			if (plist && CFGetTypeID(plist) == CFDictionaryGetTypeID()) {
				root = (CFDictionaryRef)plist;
			} else if (plist) {
				CFRelease(plist);
			}
		}
	}
	if (!root && current.valid) {
		return; // an unreadable file leaves the list as it is until it is replaced again
	}

	CFMutableArrayRef updated = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
	CFArrayRef entries = root ? (CFArrayRef)CFDictionaryGetValue(root, CFSTR("items")) : NULL;
	CFIndex count = (entries && CFGetTypeID(entries) == CFArrayGetTypeID()) ? CFArrayGetCount(entries) : 0;
	for (CFIndex i = 0; i < count; i++) {
		CFDictionaryRef entry = (CFDictionaryRef)CFArrayGetValueAtIndex(entries, i);
		if (CFGetTypeID(entry) != CFDictionaryGetTypeID()) {
			continue;
		}
		CFNumberRef number = (CFNumberRef)CFDictionaryGetValue(entry, CFSTR("id"));
		SInt64 identifier = 0;
		if (!number || CFGetTypeID(number) != CFNumberGetTypeID() || !CFNumberGetValue(number, kCFNumberSInt64Type, &identifier)) {
			continue;
		}
		CFStringRef urlString = (CFStringRef)CFDictionaryGetValue(entry, CFSTR("url"));
		CFURLRef url = (urlString && CFGetTypeID(urlString) == CFStringGetTypeID()) ? CFURLCreateWithString(alloc, urlString, NULL) : NULL;
		if (!url) {
			continue;
		}

		struct __LSSharedFileListItem *item = NULL;
		CFIndex existingCount = CFArrayGetCount(list->items);
		for (CFIndex j = 0; j < existingCount; j++) {
			struct __LSSharedFileListItem *candidate = (struct __LSSharedFileListItem *)CFArrayGetValueAtIndex(list->items, j);
			if (candidate->id == (UInt32)identifier) {
				item = (struct __LSSharedFileListItem *)CFRetain(candidate);
				break;
			}
		}
		if (!item) {
			item = __LSSharedFileListCreateItem(list, alloc);
			if (!item) {
				CFRelease(url);
				continue;
			}
			item->id = (UInt32)identifier;
		}

		if (item->url) {
			CFRelease(item->url);
		}
		item->url = url;
		if (item->displayName) {
			CFRelease(item->displayName);
			item->displayName = NULL;
		}
		CFStringRef name = (CFStringRef)CFDictionaryGetValue(entry, CFSTR("name"));
		if (name && CFGetTypeID(name) == CFStringGetTypeID()) {
			item->displayName = CFStringCreateCopy(alloc, name);
		}
		CFDictionaryRef properties = (CFDictionaryRef)CFDictionaryGetValue(entry, CFSTR("properties"));
		if (properties && CFGetTypeID(properties) == CFDictionaryGetTypeID()) {
			// Values that could not be stored stay as they are in memory.
			CFIndex propertyCount = CFDictionaryGetCount(properties);
			const void **keys = (const void **)malloc((propertyCount + 1) * sizeof(void *));
			const void **values = (const void **)malloc((propertyCount + 1) * sizeof(void *));
			if (keys && values) {
				CFDictionaryGetKeysAndValues(properties, keys, values);
				for (CFIndex k = 0; k < propertyCount; k++) {
					CFDictionarySetValue(item->properties, keys[k], values[k]);
				}
			}
			free(keys);
			free(values);
		}
		item->list = list;
		CFArrayAppendValue(updated, item);
		CFRelease(item);
	}

	CFIndex oldCount = CFArrayGetCount(list->items);
	for (CFIndex i = 0; i < oldCount; i++) {
		struct __LSSharedFileListItem *old = (struct __LSSharedFileListItem *)CFArrayGetValueAtIndex(list->items, i);
		if (!CFArrayContainsValue(updated, CFRangeMake(0, CFArrayGetCount(updated)), old)) {
			old->list = NULL;
		}
	}
	CFRelease(list->items);
	list->items = updated;

	CFDictionaryRef properties = root ? (CFDictionaryRef)CFDictionaryGetValue(root, CFSTR("properties")) : NULL;
	CFDictionaryRemoveAllValues(list->properties);
	if (properties && CFGetTypeID(properties) == CFDictionaryGetTypeID()) {
		CFIndex propertyCount = CFDictionaryGetCount(properties);
		const void **keys = (const void **)malloc((propertyCount + 1) * sizeof(void *));
		const void **values = (const void **)malloc((propertyCount + 1) * sizeof(void *));
		if (keys && values) {
			CFDictionaryGetKeysAndValues(properties, keys, values);
			for (CFIndex k = 0; k < propertyCount; k++) {
				CFDictionarySetValue(list->properties, keys[k], values[k]);
			}
		}
		free(keys);
		free(values);
	}

	CFNumberRef seed = root ? (CFNumberRef)CFDictionaryGetValue(root, CFSTR("seed")) : NULL;
	SInt64 seedValue = 0;
	if (seed && CFGetTypeID(seed) == CFNumberGetTypeID() && CFNumberGetValue(seed, kCFNumberSInt64Type, &seedValue)) {
		list->seed = (UInt32)seedValue;
	} else {
		list->seed++;
	}
	if (root) {
		CFRelease(root);
	}
}

// A new identifier for an item of the list; zero is not used and no two items share one.
static UInt32 __LSSharedFileListNewItemID(struct __LSSharedFileList *list) {
	for (;;) {
		UInt32 candidate = arc4random();
		if (candidate == 0) {
			continue;
		}
		CFIndex count = CFArrayGetCount(list->items);
		Boolean taken = false;
		for (CFIndex i = 0; i < count && !taken; i++) {
			taken = ((struct __LSSharedFileListItem *)CFArrayGetValueAtIndex(list->items, i))->id == candidate;
		}
		if (!taken) {
			return candidate;
		}
	}
}


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

	if (!list->items || !list->properties) {
		CFRelease((CFTypeRef)list);
		return NULL;
	}

	__LSSharedFileListComputePath(list);
	pthread_mutex_lock(&list->lock);
	__LSSharedFileListSyncLocked(list);
	pthread_mutex_unlock(&list->lock);

	return (LSSharedFileListRef)list;
}

CFArrayRef LSSharedFileListCopySnapshot(LSSharedFileListRef inList, UInt32 *outSnapshotSeed) {
	if (!inList) {
		return NULL;
	}

	struct __LSSharedFileList *list = (struct __LSSharedFileList *)inList;
	CFArrayRef snapshot = NULL;

	pthread_mutex_lock(&list->lock);
	__LSSharedFileListSyncLocked(list);
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

	struct __LSSharedFileListItem *item = __LSSharedFileListCreateItem(list, alloc);
	if (!item) {
		return NULL;
	}

	item->url = (CFURLRef)CFRetain(inURL);
	item->displayName = inDisplayName ? CFStringCreateCopy(alloc, inDisplayName) : NULL;

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
	int lockFile = __LSSharedFileListLockFile(list);
	__LSSharedFileListSyncLocked(list);
	if (list->items) {
		item->id = __LSSharedFileListNewItemID(list);
		CFIndex insertIdx = CFArrayGetCount(list->items);
		if (insertAfterThisItem == kLSSharedFileListItemBeforeFirst) {
			insertIdx = 0;
		} else if (insertAfterThisItem && insertAfterThisItem != kLSSharedFileListItemLast) {
			CFIndex found = 0;
			if (__LSSharedFileListFindItemLocked(list, insertAfterThisItem, &found)) {
				insertIdx = found + 1;
			}
		}
		CFArrayInsertValueAtIndex(list->items, insertIdx, item);
		list->seed++;
		__LSSharedFileListSaveLocked(list);
	}
	__LSSharedFileListUnlockFile(lockFile);
	pthread_mutex_unlock(&list->lock);

	return (LSSharedFileListItemRef)item;
}

OSStatus LSSharedFileListItemRemove(LSSharedFileListRef inList, LSSharedFileListItemRef inItem) {
	if (!inList || !inItem) {
		return paramErr;
	}

	struct __LSSharedFileList *list = (struct __LSSharedFileList *)inList;
	pthread_mutex_lock(&list->lock);
	int lockFile = __LSSharedFileListLockFile(list);
	__LSSharedFileListSyncLocked(list);
	CFIndex index = 0;
	LSSharedFileListItemRef found = __LSSharedFileListFindItemLocked(list, inItem, &index);
	if (found) {
		struct __LSSharedFileListItem *removed = (struct __LSSharedFileListItem *)CFRetain(found);
		CFArrayRemoveValueAtIndex(list->items, index);
		removed->list = NULL;
		((struct __LSSharedFileListItem *)inItem)->list = NULL;
		CFRelease(removed);
		list->seed++;
		__LSSharedFileListSaveLocked(list);
	}
	__LSSharedFileListUnlockFile(lockFile);
	pthread_mutex_unlock(&list->lock);
	return noErr;
}

OSStatus LSSharedFileListRemoveAllItems(LSSharedFileListRef inList) {
	if (!inList) {
		return paramErr;
	}

	struct __LSSharedFileList *list = (struct __LSSharedFileList *)inList;
	pthread_mutex_lock(&list->lock);
	int lockFile = __LSSharedFileListLockFile(list);
	__LSSharedFileListSyncLocked(list);
	if (list->items) {
		CFIndex count = CFArrayGetCount(list->items);
		for (CFIndex i = 0; i < count; i++) {
			((struct __LSSharedFileListItem *)CFArrayGetValueAtIndex(list->items, i))->list = NULL;
		}
		CFArrayRemoveAllValues(list->items);
		list->seed++;
		__LSSharedFileListSaveLocked(list);
	}
	__LSSharedFileListUnlockFile(lockFile);
	pthread_mutex_unlock(&list->lock);
	return noErr;
}

UInt32 LSSharedFileListGetSeedValue(LSSharedFileListRef inList) {
	if (!inList) {
		return 0;
	}
	struct __LSSharedFileList *list = (struct __LSSharedFileList *)inList;
	pthread_mutex_lock(&list->lock);
	__LSSharedFileListSyncLocked(list);
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
	__LSSharedFileListSyncLocked(list);
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
	int lockFile = __LSSharedFileListLockFile(list);
	__LSSharedFileListSyncLocked(list);
	if (inPropertyData) {
		CFDictionarySetValue(list->properties, inPropertyName, inPropertyData);
	} else {
		CFDictionaryRemoveValue(list->properties, inPropertyName);
	}
	list->seed++;
	__LSSharedFileListSaveLocked(list);
	__LSSharedFileListUnlockFile(lockFile);
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

	// The list is retained for the change so that it cannot go away while the item still points to it.
	struct __LSSharedFileList *list = item->list ? (struct __LSSharedFileList *)CFRetain((CFTypeRef)item->list) : NULL;
	if (list) {
		pthread_mutex_lock(&list->lock);
		if (item->list != list) {
			pthread_mutex_unlock(&list->lock);
			CFRelease((CFTypeRef)list);
			list = NULL;
		}
	}
	int lockFile = list ? __LSSharedFileListLockFile(list) : -1;
	if (list) {
		__LSSharedFileListSyncLocked(list);
	}
	if (inPropertyData) {
		CFDictionarySetValue(item->properties, inPropertyName, inPropertyData);
	} else {
		CFDictionaryRemoveValue(item->properties, inPropertyName);
	}
	if (list) {
		list->seed++;
		__LSSharedFileListSaveLocked(list);
		__LSSharedFileListUnlockFile(lockFile);
		pthread_mutex_unlock(&list->lock);
		CFRelease((CFTypeRef)list);
	}
	return noErr;
}
